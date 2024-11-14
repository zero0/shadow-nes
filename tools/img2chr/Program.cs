using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.IO.Compression;
using System.Net.Cache;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

#pragma warning disable CA1416

namespace img2chr
{
    class Program
    {
        [StructLayout(LayoutKind.Sequential)]
        struct SpritePalette
        {
            public Color c0;
            public Color c1;
            public Color c2;
            public Color c3;

            public Color this[int index] => index switch
            {
                0 => c0,
                1 => c1,
                2 => c2,
                3 => c3,
                _ => default,
            };
        }

        [StructLayout(LayoutKind.Sequential)]
        unsafe struct TileIndices
        {
            private fixed ushort r[8];

            public bool IsEmpty
            {
                get
                {
                    bool empty = true;
                    for (int i = 0; i < 8 && empty; ++i)
                    {
                        empty &= r[i] == 0;
                    }
                    return empty;
                }
            }

            public int this[int x, int y]
            {
                get
                {
                    Assert(x >= 0 && x < 8);
                    Assert(y >= 0 && y < 8);

                    return 0x3 & ((r[y]) >> (x * 2));
                }
                set
                {
                    Assert(x >= 0 && x < 8);
                    Assert(y >= 0 && y < 8);
                    Assert(value >= 0 && value <= 3);

                    r[y] |= (ushort)((value & 0x3) << (x * 2));
                }
            }

            public TileIndices FlipHorizontal()
            {
                TileIndices tileIndices = default;
                for (int y = 0; y < 8; ++y)
                {
                    for (int x = 0; x < 8; ++x)
                    {
                        tileIndices[x, y] = this[7 - x, y];
                    }
                }
                return tileIndices;
            }

            public TileIndices FlipVertical()
            {
                TileIndices tileIndices = default;
                for (int i = 0; i < 8; ++i)
                {
                    tileIndices.r[i] = r[7 - i];
                }
                return tileIndices;
            }

            public TileIndices FlipHorizontalVertical()
            {
                return FlipHorizontal().FlipVertical();
            }

            public override int GetHashCode()
            {
                int hash = 17;
                for (int i = 0; i < 8; ++i)
                {
                    hash = hash * 37 + r[i];
                }

                return hash;
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        struct TileEntry
        {
            public TileIndices tileIndices;
            public int paletteIndex;
            public int x, y, attr;
        }

        [StructLayout(LayoutKind.Sequential)]
        unsafe struct MetaSpriteEntry
        {
            public fixed ushort tiles[64];
            public int tileCount;
            public int x, y;

            public int this[int i] => tiles[i];

            public void Add(int index)
            {
                tiles[tileCount++] = (ushort)index;
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        struct ConvertOptions
        {
            public string defaultGeneratedAssetFolder;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct ChrRomOutput
        {
            public string chrRomAsm;
            public int chrCount;
            public bool is8x16;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct UniqueTileEntry
        {
            public int index;
            public int attr;
        }

        #region Logging
        const char esc = (char)27;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        static void LogMessage(string msg)
        {
            Console.Out.WriteLine($"{esc}[92m{msg}{esc}[0m");
        }

        //[Conditional("DEBUG")]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        static void LogInfo(string msg)
        {
            Console.Out.WriteLine(msg);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        static void ConditionalLogInfo(bool condition, string msgTrue, string msgFalse = null)
        {
            if (condition)
            {
                LogInfo(msgTrue);
            }
            else if (!string.IsNullOrEmpty(msgFalse))
            {
                LogInfo(msgFalse);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        static void LogWarning(string msg, [CallerFilePath] string filepath = "", [CallerMemberName] string memberName = "", [CallerLineNumber] int line = 0)
        {
            Console.Out.WriteLine($"{esc}[93m{filepath}({line}): {msg}{esc}[0m");
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        static void LogError(string msg, [CallerFilePath] string filepath = "", [CallerMemberName] string memberName = "", [CallerLineNumber] int line = 0)
        {
            Console.Error.WriteLine($"{esc}[91m{filepath}({line}): {msg}{esc}[0m");
        }


        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        static bool AssertWarn(bool condition, string msg = null, [CallerFilePath] string filepath = "", [CallerMemberName] string memberName = "", [CallerLineNumber] int line = 0)
        {
            if (!condition)
            {
                LogWarning(string.IsNullOrEmpty(msg) ? "Assertion failed" : msg, filepath, memberName, line);
            }
            return !condition;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        static bool Assert(bool condition, string msg = null, [CallerFilePath] string filepath = "", [CallerMemberName] string memberName = "", [CallerLineNumber] int line = 0)
        {
            if (!condition)
            {
                ExitCode = 1;
                LogError(string.IsNullOrEmpty(msg) ? "Assertion failed" : msg, filepath, memberName, line);
            }
            return !condition;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        static void InvalidCodePath(string msg = null, [CallerFilePath] string filepath = "", [CallerMemberName] string memberName = "", [CallerLineNumber] int line = 0)
        {
            ExitCode = 1;
            LogError(string.IsNullOrEmpty(msg) ? "Invalid code path" : msg, filepath, memberName, line);
        }
        #endregion

        static bool TryGetFileParamters(string srcFilename, string parameterFileExt, out Dictionary<string, string> parameters)
        {
            parameters = null;

            string spriteFilename = string.IsNullOrEmpty(parameterFileExt) ? srcFilename : $"{srcFilename}.{parameterFileExt}";
            if (File.Exists(spriteFilename))
            {
                try
                {
                    parameters = new();

                    FileStreamOptions fileOptions = new()
                    {
                        Mode = FileMode.Open,
                        Access = FileAccess.Read
                    };
                    using StreamReader sr = new StreamReader(new FileStream(spriteFilename, fileOptions));

                    string line;
                    while ((line = sr.ReadLine()) != null)
                    {
                        int idx = line.IndexOf('=');
                        if (idx > 0)
                        {
                            string key = line.Substring(0, idx).Trim();
                            string value = line.Substring(idx + 1).Trim();

                            if (!string.IsNullOrEmpty(key))
                            {
                                if (value.StartsWith('"') && value.EndsWith('"'))
                                {
                                    value = value.Substring(1, value.Length - 2);
                                }

                                parameters[key] = value;
                            }
                        }
                    }
                }
                catch (Exception e)
                {
                    LogError(e.Message);
                    parameters = null;
                }
            }

            return parameters != null;
        }

        static void WriteParametersToFile(Dictionary<string, string> parameters, string filename)
        {
            if (parameters?.Count > 0)
            {
                StringBuilder sb = new StringBuilder();
                foreach (var p in parameters)
                {
                    sb.Append(p.Key).Append('=').AppendLine(p.Value);
                }

                File.WriteAllText(filename, sb.ToString(), UTF8);
            }
        }

        static bool GetBoolParameter(Dictionary<string, string> parameters, string key, bool defaultValue = false)
        {
            bool value = parameters?.TryGetValue(key, out string strValue) ?? false ? bool.TryParse(strValue, out bool boolValue) ? boolValue : defaultValue : defaultValue;
            return value;
        }

        static int GetIntParameter(Dictionary<string, string> parameters, string key, int defaultValue = 0)
        {
            int value = defaultValue;
            if (parameters?.TryGetValue(key, out string strValue) ?? false)
            {
                System.Globalization.NumberStyles numberStyles = System.Globalization.NumberStyles.Number;
                if (strValue.StartsWith("0x", StringComparison.InvariantCultureIgnoreCase))
                {
                    numberStyles = System.Globalization.NumberStyles.HexNumber;
                    strValue = strValue.Substring(2);
                }

                if (int.TryParse(strValue, numberStyles, System.Globalization.CultureInfo.InvariantCulture, out int intValue))
                {
                    value = intValue;
                }
            }

            return value;
        }

        static long GetLongParameter(Dictionary<string, string> parameters, string key, long defaultValue = 0)
        {
            long value = defaultValue;
            if (parameters?.TryGetValue(key, out string strValue) ?? false)
            {
                System.Globalization.NumberStyles numberStyles = System.Globalization.NumberStyles.Number;
                if (strValue.StartsWith("0x", StringComparison.InvariantCultureIgnoreCase))
                {
                    numberStyles = System.Globalization.NumberStyles.HexNumber;
                    strValue = strValue.Substring(2);
                }

                if (long.TryParse(strValue, numberStyles, System.Globalization.CultureInfo.InvariantCulture, out long longValue))
                {
                    value = longValue;
                }
            }
            return value;
        }

        static char GetCharParameter(Dictionary<string, string> parameters, string key, char defaultValue)
        {
            char value = parameters?.TryGetValue(key, out string strValue) ?? false ? !string.IsNullOrEmpty(strValue) ? strValue[0] : defaultValue : defaultValue;
            return value;
        }

        static string GetStringParameter(Dictionary<string, string> parameters, string key, string defaultValue = null)
        {
            string value = parameters?.TryGetValue(key, out string strValue) ?? false ? strValue : defaultValue;
            return value;
        }

        static void SetParameter<T>(Dictionary<string, string> parameters, string key, T value, Func<T, string> convertFunc = null)
        {
            static string DefaultFunc(T v) => v.ToString();
            if (parameters != null)
            {
                parameters[key] = (convertFunc ?? DefaultFunc)(value);
            }
        }

        static string PathCombine(params string[] paths)
        {
            StringBuilder sb = new();
            if (paths?.Length > 0)
            {
                sb.Append(paths[0]);

                for (int i = 1; i < paths.Length; i++)
                {
                    sb.Append(Path.PathSeparator);
                    sb.Append(paths[i]);
                }
            }

            return sb.ToString();
        }

        static string CharacterToDefineName(char c)
        {
            string defineName = null;
            if (char.IsDigit(c))
            {
                defineName = c.ToString();
            }
            else if (char.IsLetter(c))
            {
                if (char.IsUpper(c))
                {
                    defineName = $"upper_{char.ToLower(c)}";
                }
                else
                {
                    defineName = $"lower_{c}";
                }
            }
            else
            {
                switch (c)
                {
                    case ' ': defineName = "space"; break;
                    case '.': defineName = "period"; break;
                    case ',': defineName = "comma"; break;
                    case '!': defineName = "exclimation"; break;
                    case '?': defineName = "question_mark"; break;
                    case '<': defineName = "less_than"; break;
                    case '>': defineName = "greater_than"; break;
                    case ';': defineName = "semicolon"; break;
                    case ':': defineName = "colon"; break;
                    case '[': defineName = "left_angle_bracket"; break;
                    case ']': defineName = "right_angle_bracket"; break;
                    case '@': defineName = "at"; break;
                    case '#': defineName = "hash"; break;
                    case '$': defineName = "dollar"; break;
                    case '%': defineName = "percent"; break;
                    case '^': defineName = "carot"; break;
                    case '&': defineName = "ampersand"; break;
                    case '*': defineName = "star"; break;
                    case '(': defineName = "left_parentheses"; break;
                    case ')': defineName = "right_parentheses"; break;
                    case '-': defineName = "minus"; break;
                    case '_': defineName = "underscore"; break;
                    case '=': defineName = "equals"; break;
                    case '+': defineName = "plus"; break;
                    case '~': defineName = "tilde"; break;
                    case '\'': defineName = "single_quote"; break;
                    case '`': defineName = "single_back_quote"; break;
                    case '"': defineName = "double_quote"; break;
                    case '/': defineName = "forward_slash"; break;
                    case '\\': defineName = "back_slash"; break;
                    case '\r': defineName = "carage_return"; break;
                    case '\n': defineName = "new_line"; break;
                    case '\t': defineName = "tab"; break;
                    case '\b': defineName = "break"; break;
                    default:
                        InvalidCodePath($"Unable to find name for character '{c}'");
                        break;
                }
            }

            return defineName;
        }

        delegate void ConvertFunc(string inputFilename, Dictionary<string, ChrRomOutput> outputChrData, in ConvertOptions convertOptions);

        const string uint8Type = "uint8_t";

        #region Image File
        static void ConvertImageFile(string imageFilename, Dictionary<string, ChrRomOutput> outputChrData, in ConvertOptions cmdOptions)
        {
            ConvertOptions options = cmdOptions;

            bool isSprite = TryGetFileParamters(imageFilename, "sprite", out var spriteParameters);
            bool isFont = TryGetFileParamters(imageFilename, "font", out var fontParameters);

            if (!GetBoolParameter(spriteParameters, "enable-import", true) || !GetBoolParameter(fontParameters, "enable-import", true))
            {
                return;
            }

            int metaSpriteWidth = GetIntParameter(spriteParameters, "meta-sprite.width");
            int metaSpriteHeight = GetIntParameter(spriteParameters, "meta-sprite.height");

            string filename = Path.GetFileNameWithoutExtension(imageFilename);

            FileStreamOptions fileOptions = new()
            {
                Mode = FileMode.Open,
                Access = FileAccess.Read
            };

            using var bitmapImage = new Bitmap(new FileStream(imageFilename, fileOptions));

            Assert(bitmapImage.Width >= 8, $"Image too small, width: {bitmapImage.Width} requried at least 8px");
            Assert(bitmapImage.Height >= 8, $"Image too small, height: {bitmapImage.Height} requried at least 8px");

            int spritePaletteCount = Math.Clamp(GetIntParameter(spriteParameters, "sprite.palette-count", 1), 1, 4);

            SpritePalette[] palettes = new SpritePalette[spritePaletteCount];
            for (int i = 0; i < spritePaletteCount; ++i)
            {
                // read first 4 pixels as palette
                palettes[i].c0 = bitmapImage.GetPixel(0, i);
                palettes[i].c1 = bitmapImage.GetPixel(1, i);
                palettes[i].c2 = bitmapImage.GetPixel(2, i);
                palettes[i].c3 = bitmapImage.GetPixel(3, i);
            }

            static void GetPaletteAndIndex(Color color, SpritePalette[] palettes, out int palette, out int index)
            {
                palette = -1;
                index = 0;

                for (int p = 0; p < palettes.Length; p++)
                {
                    SpritePalette spritePalette = palettes[p];
                    for (int i = 3; i >= 0; --i)
                    {
                        if (spritePalette[i] == color)
                        {
                            palette = p;
                            index = i;
                            return;
                        }
                    }
                }
            }

            List<TileEntry> allTiles = new List<TileEntry>();

            Rectangle area = new Rectangle(0, 0, bitmapImage.Width, bitmapImage.Height);

            area.X = Math.Clamp(GetIntParameter(spriteParameters, "image.max-rect.x", area.X), 0, bitmapImage.Width);
            area.Y = Math.Clamp(GetIntParameter(spriteParameters, "image.max-rect.y", area.Y), 0, bitmapImage.Height);
            area.Width = Math.Clamp(GetIntParameter(spriteParameters, "image.max-rect.w", area.Width), 0, bitmapImage.Width);
            area.Height = Math.Clamp(GetIntParameter(spriteParameters, "image.max-rect.h", area.Height), 0, bitmapImage.Height);

            Span<int> palettesUsed = stackalloc int[4];

            // read tiles (starging from 8,0 to leave room for palette info)
            for (int y = area.Y, ymax = area.Height; y < ymax; y += 8)
            {
                for (int x = y == area.Y ? area.X + 8 : area.X, xmax = area.Width; x < xmax; x += 8)
                {
                    TileIndices tileIndices = default;
                    TileIndices paletteIndices = default;

                    for (int py = 0; py < 8; ++py)
                    {
                        for (int px = 0; px < 8; ++px)
                        {
                            int vx = x + px;
                            int vy = y + py;
                            Assert(vx < bitmapImage.Width);
                            Assert(vy < bitmapImage.Height);

                            Color pixel = bitmapImage.GetPixel(vx, vy);

                            GetPaletteAndIndex(pixel, palettes, out int paletteIndex, out int pixelIndex);

                            Assert(paletteIndex >= 0, $"Unable to find palette for color {pixel} at [{vx}, {vy}]");

                            tileIndices[px, py] = pixelIndex;
                            paletteIndices[px, py] = paletteIndex;
                        }
                    }

                    // only add non-all blank tiles
                    if (!tileIndices.IsEmpty)
                    {
                        int paletteIndex = 0;

                        // clear used palettes
                        for (int p = 0; p < palettesUsed.Length; ++p)
                        {
                            palettesUsed[p] = 0;
                        }

                        // mark each used palettes
                        for (int py = 0; py < 8; ++py)
                        {
                            for (int px = 0; px < 8; ++px)
                            {
                                palettesUsed[paletteIndices[px, py]]++;
                            }
                        }

                        // make sure there's only one palette used
                        int usedCount = 0;
                        for (int p = 0; p < palettesUsed.Length; ++p)
                        {
                            usedCount += palettesUsed[p] != 0 ? 1 : 0;
                        }
                        Assert(usedCount == 1, $"More than one palette used at [{x}, {y}]. Palette Count: ({palettesUsed[0]} {palettesUsed[1]} {palettesUsed[2]} {palettesUsed[3]})");

                        // find the most common palette used
                        for (int p = 0; p < palettesUsed.Length; ++p)
                        {
                            if (palettesUsed[p] > palettesUsed[paletteIndex])
                            {
                                paletteIndex = p;
                            }
                        }

                        allTiles.Add(new()
                        {
                            x = x,
                            y = y,
                            tileIndices = tileIndices,
                            paletteIndex = paletteIndex
                        });
                    }
                }
            }

            // remove duplcate tiles
            List<int> uniqueTileIndices = new(allTiles.Count);
            List<UniqueTileEntry> allUniqueTiles = new(allTiles.Count);
            Dictionary<int, UniqueTileEntry> uniqueTiles = new(allTiles.Count);

            bool removeDuplicateSprites = GetBoolParameter(spriteParameters, "sprite.remove-duplicates", true);
            if (removeDuplicateSprites)
            {
                Dictionary<int, int> allTilesToUniqueTiles = new(allTiles.Count);
                Dictionary<int, int> uniqueTileHashToIndex = new(allTiles.Count);
                HashSet<int> uniqueTileHashes = new(allTiles.Count);

                // bucket each tile hash
                for (int i = 0; i < allTiles.Count; i++)
                {
                    TileEntry tileEntry = allTiles[i];

                    int tileHash = tileEntry.tileIndices.GetHashCode();
                    int tileHashV = tileEntry.tileIndices.FlipVertical().GetHashCode();
                    int tileHashH = tileEntry.tileIndices.FlipHorizontal().GetHashCode();
                    int tileHashVH = tileEntry.tileIndices.FlipHorizontalVertical().GetHashCode();

                    bool added = false;
                    added |= uniqueTiles.TryAdd(tileHash, new() { index = i, attr = 0 });
                    added |= uniqueTiles.TryAdd(tileHashV, new() { index = i, attr = 2 });
                    added |= uniqueTiles.TryAdd(tileHashH, new() { index = i, attr = 1 });
                    added |= uniqueTiles.TryAdd(tileHashVH, new() { index = i, attr = 3 });

                    if (added)
                    {
                        uniqueTileIndices.Add(i);
                    }
                }

                // get list of all sorted unique tiles
                for (int i = 0; i < uniqueTileIndices.Count; i++)
                {
                    TileEntry t = allTiles[uniqueTileIndices[i]];
                    int tileHash = t.tileIndices.GetHashCode();
                    allUniqueTiles.Add(uniqueTiles[tileHash]);
                }
            }

            List<MetaSpriteEntry> allMetaSprites = new List<MetaSpriteEntry>();

            // if meta sprite is used, group sprites
            if (metaSpriteWidth > 0 && metaSpriteHeight > 0)
            {
                // group tiles into sprites
                for (int y = 0, ymax = bitmapImage.Height; y < ymax; y += metaSpriteHeight)
                {
                    for (int x = 0, xmax = bitmapImage.Width; x < xmax; x += metaSpriteWidth)
                    {
                        MetaSpriteEntry metaSpriteEntry = default;
                        metaSpriteEntry.x = x;
                        metaSpriteEntry.y = y;

                        Rectangle spriteRect = new(x, y, metaSpriteWidth, metaSpriteHeight);

                        for (int i = 0; i < uniqueTileIndices.Count; i++)
                        {
                            TileEntry tileEntry = allTiles[uniqueTileIndices[i]];
                            Rectangle tileRect = new(tileEntry.x, tileEntry.y, 8, 8);

                            if (spriteRect.Contains(tileRect))
                            {
                                metaSpriteEntry.Add(uniqueTileIndices[i]);
                            }
                        }

                        allMetaSprites.Add(metaSpriteEntry);
                    }
                }
            }

            string assetLayoutSpriteKey = $"{Path.GetFileName(imageFilename)}.sprite";

            // generate text file
            const string rodataSegment = "RODATA";
            StringBuilder byte0 = new StringBuilder();
            StringBuilder byte1 = new StringBuilder();

            StringBuilder sb = new StringBuilder();
            sb.AppendLine(";");
            sb.AppendLine($"; Generated from {Path.GetFileName(imageFilename)}");
            sb.AppendLine(";");
            sb.AppendLine();

            for (int i = 0; i < palettes.Length; ++i)
            {
                sb.AppendLine($"; Palette {i}");
                sb.AppendLine($";  0: [{palettes[i].c0.R,-3}, {palettes[i].c0.G,-3}, {palettes[i].c0.B,-3}]  #{palettes[i].c0.R:X2}{palettes[i].c0.G:X2}{palettes[i].c0.B:X2}");
                sb.AppendLine($";  1: [{palettes[i].c1.R,-3}, {palettes[i].c1.G,-3}, {palettes[i].c1.B,-3}]  #{palettes[i].c1.R:X2}{palettes[i].c1.G:X2}{palettes[i].c1.B:X2}");
                sb.AppendLine($";  2: [{palettes[i].c2.R,-3}, {palettes[i].c2.G,-3}, {palettes[i].c2.B,-3}]  #{palettes[i].c2.R:X2}{palettes[i].c2.G:X2}{palettes[i].c2.B:X2}");
                sb.AppendLine($";  3: [{palettes[i].c3.R,-3}, {palettes[i].c3.G,-3}, {palettes[i].c3.B,-3}]  #{palettes[i].c3.R:X2}{palettes[i].c3.G:X2}{palettes[i].c3.B:X2}");
                sb.AppendLine();
            }

            sb.AppendLine("; Tiles");
            sb.AppendLine($";     All: {allTiles.Count}");
            sb.AppendLine($";  Unique: {allUniqueTiles.Count}");
            sb.AppendLine($";    Size: {allUniqueTiles.Count * 8 * 2:N0}b");
            sb.AppendLine();

            string exportName = filename.Replace(' ', '_').Replace('-', '_');
            sb.AppendLine($".export _{exportName}");
            sb.AppendLine($"_{exportName}:");
            sb.AppendLine();

            // convert tiles to bytes
            for (int i = 0; i < allUniqueTiles.Count; i++)
            {
                UniqueTileEntry uniqueTileEntry = allUniqueTiles[i];
                TileEntry tileEntry = allTiles[uniqueTileEntry.index];

                byte0.Clear();
                byte1.Clear();

                for (int y = 0; y < 8; ++y)
                {
                    byte0.Append(".byte %");
                    byte1.Append(".byte %");

                    for (int x = 0; x < 8; ++x)
                    {
                        byte0.Append((tileEntry.tileIndices[x, y] & 0x1) == 0x1 ? "1" : "0");
                        byte1.Append((tileEntry.tileIndices[x, y] & 0x2) == 0x2 ? "1" : "0");
                    }

                    byte0.AppendLine();
                    byte1.AppendLine();
                }

                sb.AppendLine($"; Tile {i} {tileEntry.x}x{tileEntry.y}");
                sb.AppendLine($"{exportName}_{tileEntry.x}x{tileEntry.y}:");
                sb.AppendLine(byte0.ToString());
                sb.AppendLine(byte1.ToString());
            }

            sb.AppendLine();

            // output chr data
            outputChrData.Add(assetLayoutSpriteKey, new()
            {
                chrRomAsm = sb.ToString(),
                chrCount = allUniqueTiles.Count,
                is8x16 = false
            });

            // clear for meta sprite info
            sb.Clear();

            // write meta sprites
            if (allMetaSprites.Count > 0)
            {
                int totalMetaSpriteTiles = 0;
                for (int i = 0; i < allMetaSprites.Count; i++)
                {
                    MetaSpriteEntry metaSprite = allMetaSprites[i];
                    totalMetaSpriteTiles += metaSprite.tileCount;
                }

                sb.AppendLine(";");
                sb.AppendLine("; Meta-Sprites");
                sb.AppendLine($";  Dimenstions: {metaSpriteWidth}x{metaSpriteHeight}");
                sb.AppendLine($";        Count: {allMetaSprites.Count}");
                sb.AppendLine($";         Size: {(allMetaSprites.Count * 3) + (totalMetaSpriteTiles * 2):N0}b");
                sb.AppendLine(";       Format: .byte [tile_base_address hi] [tile_base_address lo] [tile count * 2] {attr:%HVXXXYYY tile-index:%PPIIIIII} ...");
                sb.AppendLine(";             : V - flip vertical; H - flip horizontal");
                sb.AppendLine(";             : XXX - tile offset X from 0,0 of meta-sprite; YYY - tile offset Y from 0,0 of meta-sprite");
                sb.AppendLine(";             : PP - tile palette to use for the meta-sprite; IIIIII - tile index from [base address]");
                sb.AppendLine();
                sb.AppendLine($".segment \"{rodataSegment}\"");
                sb.AppendLine();

                // write meta sprite format
                for (int i = 0; i < allMetaSprites.Count; i++)
                {
                    MetaSpriteEntry metaSprite = allMetaSprites[i];
                    if (metaSprite.tileCount > 0)
                    {
                        int minTileIndex = int.MaxValue;
                        for (int s = 0; s < metaSprite.tileCount; s++)
                        {
                            int tileHash = allTiles[metaSprite[s]].tileIndices.GetHashCode();
                            UniqueTileEntry uniqueTileEntry = uniqueTiles[tileHash];
                            minTileIndex = minTileIndex < uniqueTileEntry.index ? minTileIndex : uniqueTileEntry.index;
                        }

                        sb.AppendLine($"; Meta-Sprite Tile Count: {metaSprite.tileCount}   Meta-Sprite Min Tile Index: {minTileIndex}");
                        sb.AppendLine($"_{exportName}_sprite_{i}:");
                        sb.AppendLine($".export _{exportName}_sprite_{i}");

                        TileEntry minTile = allTiles[minTileIndex];
                        sb.AppendLine($".addr {exportName}_{minTile.x}x{minTile.y}");

                        int byteCount = (metaSprite.tileCount * 2) + 3;
                        Assert(byteCount <= 0xFF, $"Meta-sprite size is too large {byteCount}b");

                        // include [attr, tile] pairs + ptr and size bytes so asm code is easier to use
                        sb.AppendLine($".byte {byteCount} ; byte count (including 2 byte address and 1 byte size)");

                        for (int s = 0; s < metaSprite.tileCount; s++)
                        {
                            int tileHash = allTiles[metaSprite[s]].tileIndices.GetHashCode();
                            UniqueTileEntry uniqueTileEntry = uniqueTiles[tileHash];
                            TileEntry tileEntry = allTiles[uniqueTileEntry.index];

                            int rx = tileEntry.x - metaSprite.x;
                            int ry = tileEntry.y - metaSprite.y;
                            Assert(rx >= 0, $"({i}) @ ({metaSprite[s]}) {rx} (rx) >= 0");
                            Assert(ry >= 0, $"({i}) @ ({metaSprite[s]}) {ry} (ry) >= 0");

                            int attr = (0x3 & uniqueTileEntry.attr) << 6;
                            attr |= (0x7 & (rx >> 3)) << 3;
                            attr |= (0x7 & (ry >> 3)) << 0;

                            int idx = uniqueTileEntry.index - minTileIndex;
                            Assert(idx <= 0x3F, $"Index out of range: {idx}");
                            idx &= 0x3F;

                            int pal = (0x03 & tileEntry.paletteIndex) << 6;

                            sb.AppendLine($".byte ${attr:X2}, {pal | idx}");
                        }

                        sb.AppendLine();
                    }
                }
            }

            // output meta sprite
            if (sb.Length > 0)
            {
                string sFilename = Path.Combine(Path.GetDirectoryName(imageFilename), cmdOptions.defaultGeneratedAssetFolder, "asm", $"{filename}.s");
                File.WriteAllText(sFilename, sb.ToString(), UTF8);
                LogInfo($"Converted {imageFilename} -> {sFilename}");
            }

            // TODO: remove this as the chr layout file should handle this
            // Generate sprite header file
            if (false)
            {
                sb.Clear();

                sb.AppendLine("//");
                sb.AppendLine($"// Generated from {Path.GetFileName(imageFilename)}");
                sb.AppendLine("//");
                sb.AppendLine();

                string defineGuard = $"{filename}_sprite_h".Replace('-', '_').Replace('.', '_').ToUpperInvariant();
                sb.AppendLine($"#ifndef {defineGuard}");
                sb.AppendLine($"#define {defineGuard}");
                sb.AppendLine();

                //sb.AppendLine($"#define {$"{exportName.ToUpper()}_CHR_ROM",-40} ({uint8Type})({chrRomSegmentIndex})");

                for (int i = 0; i < allUniqueTiles.Count; i++)
                {
                    UniqueTileEntry uniqueTileEntry = allUniqueTiles[i];
                    TileEntry tileEntry = allTiles[uniqueTileEntry.index];

                    //sb.AppendLine($"#define {$"{exportName.ToUpper()}_{tileEntry.x}x{tileEntry.y}_CHR_ROM",-40} ({uint8Type})({chrRomSegmentIndex})");
                }

                // TODO: support meta sprites being on different CHR ROM segments

                sb.AppendLine();
                sb.AppendLine($"#endif // {defineGuard}");

                // write font file
                string hFilename = Path.Combine(Path.GetDirectoryName(imageFilename), filename) + ".sprite.h";
                File.WriteAllText(hFilename, sb.ToString(), UTF8);
                LogInfo($"Converted {imageFilename} -> {hFilename}");
            }

            // if the image is also a font, do some extra processing
            if (isFont)
            {
                string charMap = GetStringParameter(fontParameters, "charmap");
                Assert(!string.IsNullOrEmpty(charMap), "Font requires 'charmap' property");
                if (!string.IsNullOrEmpty(charMap))
                {
                    // Gemerate charmap .s file
                    {
                        sb.Clear();

                        sb.AppendLine(";");
                        sb.AppendLine($"; Generated from {Path.GetFileName(imageFilename)}");
                        sb.AppendLine(";");
                        sb.AppendLine();

                        sb.AppendLine(".feature string_escapes +");
                        sb.AppendLine();

                        sb.AppendLine("; Char Map");

                        int charOffset = GetIntParameter(fontParameters, "charmap.offset", 0);

                        int index = 0;
                        for (int i = 0; i < charMap.Length; ++i, ++index)
                        {
                            char c = charMap[i];
                            sb.AppendLine($".charmap {(int)c,-3}, {charOffset + index,-2} ; {c}");
                        }

                        string charMapExtra = GetStringParameter(fontParameters, "charmap.extra");
                        if (!string.IsNullOrEmpty(charMapExtra))
                        {
                            sb.AppendLine();
                            sb.AppendLine("; Extra Char Map");

                            string str = string.Empty;
                            for (int i = 0; i < charMapExtra.Length; ++i, ++index)
                            {
                                char c = charMapExtra[i];
                                if (c == '\\')
                                {
                                    ++i;
                                    c = charMapExtra[i];
                                    switch (c)
                                    {
                                        case 'n': c = '\n'; str = "\\n"; break;
                                        case 'r': c = '\r'; str = "\\r"; break;
                                        case 't': c = '\t'; str = "\\t"; break;
                                        case 'b': c = '\b'; str = "\\b"; break;
                                        case '\\': c = '\\'; str = "\\"; break;
                                        default: InvalidCodePath($"Invalid escape character {c}"); break;
                                    }
                                }
                                else if (c == ' ')
                                {
                                    str = "<space>";
                                }

                                sb.AppendLine($".charmap {(int)c,-3}, {charOffset + index,-2} ; {str}");
                            }
                        }

                        // write font file
                        string sFilename = Path.Combine(Path.GetDirectoryName(imageFilename), cmdOptions.defaultGeneratedAssetFolder, "asm", $"{filename}.font.s");
                        File.WriteAllText(sFilename, sb.ToString(), UTF8);
                        LogInfo($"Converted {imageFilename} -> {sFilename}");
                    }

                    // Generate charmap header file
                    {
                        sb.Clear();

                        sb.AppendLine("//");
                        sb.AppendLine($"// Generated from {Path.GetFileName(imageFilename)}");
                        sb.AppendLine("//");
                        sb.AppendLine();

                        string defineGuard = $"{filename}_font_h".Replace('-', '_').Replace('.', '_').ToUpperInvariant();
                        sb.AppendLine($"#ifndef {defineGuard}");
                        sb.AppendLine($"#define {defineGuard}");
                        sb.AppendLine();

                        int charOffset = GetIntParameter(fontParameters, "charmap.offset", 0);

                        int index = 0;
                        for (int i = 0; i < charMap.Length; ++i, ++index)
                        {
                            char c = charMap[i];
                            string defineName = CharacterToDefineName(c);

                            if (!string.IsNullOrEmpty(defineName))
                            {
                                sb.AppendLine($"#define {$"FONT_CHAR_{defineName.ToUpperInvariant()}",-40} ({uint8Type})({charOffset + index})");
                            }
                        }

                        string charMapExtra = GetStringParameter(fontParameters, "charmap.extra");
                        if (!string.IsNullOrEmpty(charMapExtra))
                        {
                            for (int i = 0; i < charMapExtra.Length; ++i, ++index)
                            {
                                char c = charMapExtra[i];
                                if (c == '\\')
                                {
                                    ++i;
                                    c = charMapExtra[i];
                                    switch (c)
                                    {
                                        case 'n': c = '\n'; break;
                                        case 'r': c = '\r'; break;
                                        case 't': c = '\t'; break;
                                        case 'b': c = '\b'; break;
                                        case '\\': c = '\\'; break;
                                        default: InvalidCodePath($"Invalid escape character {c}"); break;
                                    }
                                }

                                string defineName = CharacterToDefineName(c);

                                if (!string.IsNullOrEmpty(defineName))
                                {
                                    sb.AppendLine($"#define {$"FONT_CHAR_{defineName.ToUpperInvariant()}",-40} ({uint8Type})({charOffset + index})");
                                }
                            }
                        }

                        sb.AppendLine();
                        sb.AppendLine($"#endif // {defineGuard}");

                        // write font file
                        string hFilename = Path.Combine(Path.GetDirectoryName(imageFilename), cmdOptions.defaultGeneratedAssetFolder, "include", $"{filename}.font.h");
                        File.WriteAllText(hFilename, sb.ToString(), UTF8);
                        LogInfo($"Converted {imageFilename} -> {hFilename}");
                    }
                }
            }

        }
        #endregion

        #region Aseprite File
        enum ChunkType : ushort
        {
            None = 0,
            OldPaletteChunk1 = 0x0004,
            OldPaletteChunk2 = 0x0011,
            LayerChunk = 0x2004,
            CelChunk = 0x2005,
            CelExtraChunk = 0x2006,
            ColorProfileChunk = 0x2007,
            ExternalFilesChunk = 0x2008,
            MaskChunk = 0x2016, // Deprecated
            PathChunk = 0x2017,
            TagsChunk = 0x2018,
            PaletteChunk = 0x2019,
            UserDataChunk = 0x2020,
            SliceChunk = 0x2022,
            TilesetChunk = 0x2023,
        };

        enum ColorDepth
        {
            Palette = 8,
            Grayscale = 16,
            RGBA = 32,
        }

        enum ColorProfileType
        {
            None,
            sRGB,
            ICC,
        };

        [Flags]
        enum ColorProfileFlags
        {
            None = 0,
            UseFixedGamma = 1 << 0,
        }

        [Flags]
        enum PaletteFlags
        {
            None = 0,
            HasName = 1 << 0,
        }

        enum ExternalFileType
        {
            Palette,
            Tileset,
            ExtensionNameForProperties,
            ExtensionNameForTileManagement,
        }
        class ExternalFile
        {
            public string filename;
            public uint entryID;
            public ExternalFileType type;
        }

        [Flags]
        enum UserDataFlags
        {
            None = 0,
            HasText = 1 << 0,
            HasColor = 1 << 1,
            HasProperties = 1 << 2,
        }

        class UserData
        {
            public string text;
            public PixelRGBA color;
            public PropertyMap[] propertyMaps;
            public byte[] propertyMapData;

            public class PropertyMap
            {
                public enum PropertyType
                {
                    None,
                    Boolean,
                    Int8,
                    UInt8,
                    Int16,
                    UInt16,
                    Int32,
                    UInt32,
                    Int64,
                    UInt64,
                    Fixed,
                    Float32,
                    Float64,
                    String,
                    Point,
                    Size,
                    Rect,
                    Vector,
                    PropertyMap,
                    UUID,
                }
                public struct Property
                {
                    public PropertyType type;
                    public int offset;
                }

                public uint key;
                public Dictionary<string, Property> properties;
            }
        }

        enum CelType
        {
            RawImage,
            LinkedCel,
            CompressedImage,
            CompressedTilemap,
        };

        [Flags]
        enum CelExtraFlags
        {
            None = 0,
            PreciseBoundsSet = 1 << 0
        }

        class Cel
        {
            public ushort layerIndex;
            public short xPos;
            public short yPos;
            public byte opacityLevel;
            public CelType type;
            public short zIndex;

            public PixelData pixelData;
            public LinkedCel linkedCel;
            public Tilemap tilemap;

            public CelExtraFlags celExtraFlags;

            public FixedPoint preciseXPos;
            public FixedPoint preciseYPos;
            public FixedPoint preciseWidth;
            public FixedPoint preciseHeight;

            public List<UserData> userData;

            public class PixelData
            {
                // pixel data
                public ushort width;
                public ushort height;
                public byte[] pixelData;
#if false
                unsafe public Span<PixelRGBA> pixelDataRGBA
                {
                    get
                    {
                        Assert(pixelData.Length % sizeof(PixelRGBA) == 0);
                        fixed (void* ptr = pixelData)
                        {
                            return new Span<PixelRGBA>(ptr, pixelData.Length / sizeof(PixelRGBA));
                        }
                    }
                }

                unsafe public Span<PixelGrayscale> pixelDataGrayscale
                {
                    get
                    {
                        Assert(pixelData.Length % sizeof(PixelGrayscale) == 0);
                        fixed (void* ptr = pixelData)
                        {
                            return new Span<PixelGrayscale>(ptr, pixelData.Length / sizeof(PixelGrayscale));
                        }
                    }
                }

                unsafe public Span<PixelIndexed> pixelDataIndexed
                {
                    get
                    {
                        Assert(pixelData.Length % sizeof(PixelIndexed) == 0);
                        fixed (void* ptr = pixelData)
                        {
                            return new Span<PixelIndexed>(ptr, pixelData.Length / sizeof(PixelIndexed));
                        }
                    }
                }
#endif
            }

            public class LinkedCel
            {
                public int frameToLink;
            }

            public class Tilemap
            {
                public ushort width;
                public ushort height;
                public ushort bitsPerTile;

                public uint maskTileID;
                public uint maskXFlip;
                public uint maskYFlip;
                public uint maskDiagonalFlip;
                public byte[] tileData;

                unsafe public Span<byte> tileDataUInt8
                {
                    get
                    {
                        return new Span<byte>(tileData);
                    }
                }

                unsafe public Span<ushort> tileDataUInt16
                {
                    get
                    {
                        fixed (void* ptr = tileData)
                        {
                            return new Span<ushort>(ptr, tileData.Length / sizeof(ushort));
                        }
                    }
                }

                unsafe public Span<uint> tileDataUInt32
                {
                    get
                    {
                        fixed (void* ptr = tileData)
                        {
                            return new Span<uint>(ptr, tileData.Length / sizeof(uint));
                        }
                    }
                }
            }

        }

        enum LayerType
        {
            Normal,
            Group,
            Tilemap,
        }

        [Flags]
        enum LayerFlags
        {
            None = 0,
            Visible = 1 << 0,
            Editable = 1 << 1,
            LockMovement = 1 << 2,
            Background = 1 << 3,
            PreferLinkedCels = 1 << 4,
            LayerGroupCollapsed = 1 << 5,
            ReferenceLayer = 1 << 6,
        }

        enum BlendMode
        {
            Normal = 0,
            Multiply = 1,
            Screen = 2,
            Overlay = 3,
            Darken = 4,
            Lighten = 5,
            ColorDodge = 6,
            ColorBurn = 7,
            HardLight = 8,
            SoftLight = 9,
            Difference = 10,
            Exclusion = 11,
            Hue = 12,
            Saturation = 13,
            Color = 14,
            Luminosity = 15,
            Addition = 16,
            Subtract = 17,
            Divide = 18,
        }

        class Palette
        {
            public PaletteColorRGBA color;
            public string name;

            public List<UserData> userData;
        }

        class Layer
        {
            public LayerFlags flags;
            public LayerType type;
            public ushort layerChildLevel;
            public ushort defaultWidthInPixels;
            public ushort defaultHeightInPixels;
            public BlendMode blendMode;
            public byte opacity;
            public string name;
            public uint tilesetIndex;
            public List<UserData> userData;
        };

        class Tag
        {
            public ushort fromFrame;
            public ushort toFrame;
            public byte loopAnimDirection;
            public ushort repeatCount;
            public PaletteColorRGB tagColor;
            public string tagName;
            public List<UserData> userData;
        }

        [Flags]
        enum SliceFlags
        {
            None = 0,
            NineSlice = 1 << 0,
            HasPivot = 1 << 1,
        }

        class Slice
        {
            public string name;

            public Key[] sliceKeys;
            public List<UserData> userData;

            public struct Key
            {
                public uint frameNumber;
                public int sliceXOrigin;
                public int sliceYOrigin;
                public uint sliceWidth;
                public uint sliceHeight;
                public int centerXPos;
                public int centerYPos;
                public uint centerWidth;
                public uint centerHeight;
                public int pivotXPos;
                public int pivotYPos;
            }
        }

        [Flags]
        enum TilesetFlags
        {
            None,
            IncludeExternalFile = 1 << 0,
            IncludeTilesInFile = 1 << 1,
            UseEmptyTile0 = 1 << 2,
            MatchXFlip = 1 << 3,
            MatchYFlip = 1 << 4,
            MatchDFlip = 1 << 5,
        }

        class Tileset
        {
            public uint tilesetID;
            public TilesetFlags flags;
            public uint numTiles;
            public ushort tileWidth;
            public ushort tileHeight;
            public short baseIndex;
            public string name;
            public uint externalFileID;
            public uint externalTilesetID;
            public byte[] tilesetData;
            public List<UserData> userData;
        }

        [StructLayout(LayoutKind.Sequential, Size = 4)]
        struct FixedPoint
        {
            public short number;
            public short fraction;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct Point
        {
            public int x;
            public int y;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct Size
        {
            public int width;
            public int height;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct Rect
        {
            public Point origin;
            public Size size;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct PaletteColorRGB
        {
            public byte r, g, b;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct PaletteColorRGBA
        {
            public byte r, g, b, a;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct PixelRGBA
        {
            public byte r, g, b, a;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct PixelGrayscale
        {
            public byte value, a;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct PixelIndexed
        {
            public byte index;
        }

        //
        // Format Reference: https://github.com/aseprite/aseprite/blob/main/docs/ase-file-specs.md
        //
        static void ConverteAsepriteFile(string asepriteFilename, Dictionary<string, ChrRomOutput> outputChrData, in ConvertOptions cmdOptions)
        {
            ConvertOptions options = cmdOptions;

            bool isSprite = TryGetFileParamters(asepriteFilename, "sprite", out var spriteParameters);

            if (!GetBoolParameter(spriteParameters, "enable-import", true))
            {
                return;
            }

            byte[] asepriteData = null;
            try
            {
                asepriteData = File.ReadAllBytes(asepriteFilename);
            }
            catch (Exception e)
            {
                LogError(e.Message);
            }

            if (asepriteData?.Length == 0)
            {
                return;
            }

            Dictionary<uint, ExternalFile> externalFiles = new();
            List<Palette> palettes = new();
            List<Layer> layers = new();
            List<Cel> cels = new();
            List<Tag> tags = new();
            List<Slice> slices = new();
            List<Tileset> tilesets = new();

            #region Process Aseprite Formal
            const ushort MagicNumberHeader = 0xA5E0;
            const ushort MagicNumberFrame = 0xF1FA;

            using MemoryStream ms = new MemoryStream(asepriteData, false);

            // header
            AssertReadMemoryEquals<uint>(ms, (uint)asepriteData.Length);
            AssertReadMemoryEquals<ushort>(ms, (ushort)MagicNumberHeader);
            ushort frames = ReadMemory<ushort>(ms);
            ushort widthPixels = ReadMemory<ushort>(ms);
            ushort heightPixels = ReadMemory<ushort>(ms);
            ColorDepth colorDepth = (ColorDepth)ReadMemory<ushort>(ms);
            uint flags = ReadMemory<uint>(ms);
            ushort speedMS = ReadMemory<ushort>(ms);
            AssertReadMemoryEquals<uint>(ms, 0); // 0
            AssertReadMemoryEquals<uint>(ms, 0); // 0
            byte paletteEntry = ReadMemory<byte>(ms);
            IgnoreMemory<byte>(ms); // ignore
            IgnoreMemory<byte>(ms); // ignore
            IgnoreMemory<byte>(ms); // ignore
            ushort numColors = ReadMemory<ushort>(ms);
            byte pixelWidth = ReadMemory<byte>(ms);
            byte pixelHeight = ReadMemory<byte>(ms);
            short xPositionOfGrid = ReadMemory<short>(ms);
            short yPositionOfGrid = ReadMemory<short>(ms);
            ushort gridWidth = ReadMemory<ushort>(ms);
            ushort gridHeight = ReadMemory<ushort>(ms);
            IgnoreMemory<byte>(ms, 84);

            // frames
            for (int f = 0, fmax = frames; f < fmax; ++f)
            {
                uint frameSize = ReadMemory<uint>(ms);
                AssertReadMemoryEquals<ushort>(ms, MagicNumberFrame);
                ushort numChunksOld = ReadMemory<ushort>(ms);
                ushort frameDurationMS = ReadMemory<ushort>(ms);
                IgnoreMemory<byte>(ms, 2);
                uint numChunksNew = ReadMemory<uint>(ms);

                uint numChunks = numChunksNew == 0 ? numChunksOld : numChunksNew;
                bool ignoreOldPalette = false;

                Cel currentCel = null;
                ChunkType lastReadChunk = ChunkType.None;

                // chunks
                for (int c = 0, cmax = (int)numChunks; c < cmax; ++c)
                {
                    uint chunckSize = ReadMemory<uint>(ms);
                    ChunkType chunkType = (ChunkType)ReadMemory<ushort>(ms);

                    // chunk size includes header
                    int chunchReadSize = (int)chunckSize - (sizeof(uint) + sizeof(ushort));

                    if (chunchReadSize > 0)
                    {
                        switch (chunkType)
                        {
                            case ChunkType.None:
                                InvalidCodePath("No chunk read");
                                break;
                            case ChunkType.OldPaletteChunk1:
                                {
                                    if (!ignoreOldPalette)
                                    {
                                        ushort numPackets = ReadMemory<ushort>(ms);
                                        for (int p = 0, pmax = (int)numPackets; p < pmax; ++p)
                                        {
                                            byte paletteEntriesToSkip = ReadMemory<byte>(ms);
                                            byte numColorsInPacket = ReadMemory<byte>(ms);

                                            for (int cp = 0, cpmax = numColorsInPacket == 0 ? 256 : numColorsInPacket; c < cmax; ++c)
                                            {
                                                PaletteColorRGB paletteColor = ReadMemory<PaletteColorRGB>(ms);

                                                int idx = cp + paletteEntriesToSkip;
                                                while (palettes.Count <= idx)
                                                {
                                                    palettes.Add(new()
                                                    {
                                                        color = default,
                                                        name = "<unknown>"
                                                    });
                                                }

                                                palettes[idx].color = new()
                                                {
                                                    r = paletteColor.r,
                                                    g = paletteColor.g,
                                                    b = paletteColor.b,
                                                    a = 255,
                                                };
                                            }
                                        }
                                    }
                                }
                                break;
                            case ChunkType.OldPaletteChunk2:
                                {
                                    if (!ignoreOldPalette)
                                    {
                                        ushort numPackets = ReadMemory<ushort>(ms);
                                        for (int p = 0, pmax = (int)numPackets; p < pmax; ++p)
                                        {
                                            byte paletteEntriesToSkip = ReadMemory<byte>(ms);
                                            byte numColorsInPacket = ReadMemory<byte>(ms);

                                            for (int cp = 0, cpmax = numColorsInPacket == 0 ? 256 : numColorsInPacket; c < cmax; ++c)
                                            {
                                                PaletteColorRGB paletteColor = ReadMemory<PaletteColorRGB>(ms);

                                                int idx = cp + paletteEntriesToSkip;
                                                while (palettes.Count <= idx)
                                                {
                                                    palettes.Add(new()
                                                    {
                                                        color = default,
                                                        name = "<unknown>"
                                                    });
                                                }

                                                // [0..63] -> [0..255]
                                                palettes[idx].color = new()
                                                {
                                                    r = (byte)(paletteColor.r * 4),
                                                    g = (byte)(paletteColor.g * 4),
                                                    b = (byte)(paletteColor.b * 4),
                                                    a = 255,
                                                };
                                            }
                                        }
                                    }
                                }
                                break;
                            case ChunkType.LayerChunk:
                                {
                                    LayerFlags layerFlags = (LayerFlags)ReadMemory<ushort>(ms);
                                    LayerType layerType = (LayerType)ReadMemory<ushort>(ms);
                                    ushort layerChildLevel = ReadMemory<ushort>(ms);
                                    ushort defaultLayerWidthInPixels = ReadMemory<ushort>(ms);
                                    ushort defaultLayerHeightInPixels = ReadMemory<ushort>(ms);
                                    BlendMode blendMode = (BlendMode)ReadMemory<ushort>(ms);
                                    byte opacity = ReadMemory<byte>(ms);
                                    IgnoreMemory<byte>(ms, 3);
                                    string layerName = ReadString(ms);
                                    uint tilesetIndex = 0;

                                    if (layerType == LayerType.Tilemap)
                                    {
                                        tilesetIndex = ReadMemory<uint>(ms);
                                    }

                                    layers.Add(new()
                                    {
                                        flags = layerFlags,
                                        type = layerType,
                                        layerChildLevel = layerChildLevel,
                                        defaultWidthInPixels = defaultLayerWidthInPixels,
                                        defaultHeightInPixels = defaultLayerHeightInPixels,
                                        blendMode = blendMode,
                                        opacity = opacity,
                                        name = layerName,
                                        tilesetIndex = tilesetIndex,
                                    });
                                }
                                break;
                            case ChunkType.CelChunk:
                                {
                                    ushort layerIndex = ReadMemory<ushort>(ms);
                                    short xPos = ReadMemory<short>(ms);
                                    short yPos = ReadMemory<short>(ms);
                                    byte opacity = ReadMemory<byte>(ms);
                                    CelType celType = (CelType)ReadMemory<ushort>(ms);
                                    short zIndex = ReadMemory<short>(ms);
                                    IgnoreMemory<byte>(ms, 5);

                                    currentCel = new();
                                    currentCel.layerIndex = layerIndex;
                                    currentCel.xPos = xPos;
                                    currentCel.yPos = yPos;
                                    currentCel.opacityLevel = opacity;
                                    currentCel.type = celType;
                                    currentCel.zIndex = zIndex;

                                    switch (celType)
                                    {
                                        case CelType.RawImage:
                                            {
                                                Cel.PixelData pixelData = new();
                                                pixelData.width = ReadMemory<ushort>(ms);
                                                pixelData.height = ReadMemory<ushort>(ms);

                                                int readSize = pixelData.width * pixelData.height * (int)colorDepth;
                                                pixelData.pixelData = ReadMemory(ms, readSize);

                                                currentCel.pixelData = pixelData;
                                            }
                                            break;
                                        case CelType.LinkedCel:
                                            {
                                                Cel.LinkedCel linkedCel = new()
                                                {
                                                    frameToLink = ReadMemory<ushort>(ms)
                                                };

                                                currentCel.linkedCel = linkedCel;
                                            }
                                            break;
                                        case CelType.CompressedImage:
                                            {
                                                currentCel.pixelData = new();
                                                currentCel.pixelData.width = ReadMemory<ushort>(ms);
                                                currentCel.pixelData.height = ReadMemory<ushort>(ms);

                                                using (ZLibStream compressedStream = new ZLibStream(ms, CompressionMode.Decompress, true))
                                                {
                                                    int readSize = currentCel.pixelData.width * currentCel.pixelData.height * (int)colorDepth;
                                                    currentCel.pixelData.pixelData = DecompressMemory(compressedStream, 0, readSize);
                                                }
                                            }
                                            break;
                                        case CelType.CompressedTilemap:
                                            {
                                                Cel.Tilemap tilemap = new();
                                                tilemap.width = ReadMemory<ushort>(ms);
                                                tilemap.height = ReadMemory<ushort>(ms);
                                                tilemap.bitsPerTile = ReadMemory<ushort>(ms);
                                                tilemap.maskTileID = ReadMemory<uint>(ms);
                                                tilemap.maskXFlip = ReadMemory<uint>(ms);
                                                tilemap.maskYFlip = ReadMemory<uint>(ms);
                                                tilemap.maskDiagonalFlip = ReadMemory<uint>(ms);
                                                IgnoreMemory<byte>(ms, 10);

                                                using (ZLibStream compressedStream = new ZLibStream(ms, CompressionMode.Decompress, true))
                                                {
                                                    int readSize = tilemap.width * tilemap.height * tilemap.bitsPerTile;
                                                    tilemap.tileData = DecompressMemory(compressedStream, 0, readSize);
                                                }

                                                currentCel.tilemap = tilemap;
                                            }
                                            break;
                                        default:
                                            InvalidCodePath($"Unknown Cel Type {celType}");
                                            break;
                                    }

                                    cels.Add(currentCel);
                                }
                                break;
                            case ChunkType.CelExtraChunk:
                                {
                                    // TODO: finish implementing
                                    Assert(currentCel != null);
                                    uint celExtraFlags = ReadMemory<uint>(ms);
                                    FixedPoint preciseXPos = ReadMemory<FixedPoint>(ms);
                                    FixedPoint preciseYPos = ReadMemory<FixedPoint>(ms);
                                    FixedPoint widthOfCellInSprite = ReadMemory<FixedPoint>(ms);
                                    FixedPoint heightOfCellInSprite = ReadMemory<FixedPoint>(ms);
                                    IgnoreMemory<byte>(ms, 16);
                                }
                                break;
                            case ChunkType.ColorProfileChunk:
                                {
                                    ColorProfileType type = (ColorProfileType)ReadMemory<ushort>(ms);
                                    ColorProfileFlags colorProfileFlags = (ColorProfileFlags)ReadMemory<ushort>(ms);
                                    FixedPoint fixedGamma = ReadMemory<FixedPoint>(ms);
                                    IgnoreMemory<byte>(ms, 8);

                                    if (type == ColorProfileType.ICC)
                                    {
                                        uint iccDataLength = ReadMemory<uint>(ms);
                                        byte[] iccData = ReadMemory(ms, (int)iccDataLength);
                                    }
                                }
                                break;
                            case ChunkType.ExternalFilesChunk:
                                {
                                    uint numEntries = ReadMemory<uint>(ms);
                                    IgnoreMemory<byte>(ms, 8);

                                    for (int e = 0, emax = (int)numEntries; e < emax; ++e)
                                    {
                                        uint entryID = ReadMemory<uint>(ms);
                                        byte type = ReadMemory<byte>(ms);
                                        IgnoreMemory<byte>(ms, 7);
                                        string externalFileName = ReadString(ms);

                                        bool added = externalFiles.TryAdd(entryID, new()
                                        {
                                            filename = externalFileName,
                                            entryID = entryID,
                                            type = (ExternalFileType)type
                                        });
                                        Assert(added, $"Unable to add External File {externalFileName} at {entryID}");
                                    }
                                }
                                break;
                            case ChunkType.MaskChunk:
                                {
                                    Assert(false, "MaskChuck as been deprecated");
                                    IgnoreMemory<short>(ms);
                                    IgnoreMemory<short>(ms);
                                    ushort width = ReadMemory<ushort>(ms);
                                    ushort height = ReadMemory<ushort>(ms);
                                    IgnoreMemory<byte>(ms, 8);
                                    ushort maskNameLength = ReadMemory<ushort>(ms);
                                    IgnoreMemory<byte>(ms, maskNameLength);
                                    IgnoreMemory<byte>(ms, height * ((width + 7) / 8));
                                }
                                break;
                            case ChunkType.PathChunk:
                                {
                                    Assert(false, "PathChunk is never used");
                                }
                                break;
                            case ChunkType.TagsChunk:
                                {
                                    ushort numTags = ReadMemory<ushort>(ms);
                                    IgnoreMemory<byte>(ms, 8);

                                    for (int t = 0, tmax = numTags; t < tmax; ++t)
                                    {
                                        Tag tag = new();

                                        tag.fromFrame = ReadMemory<ushort>(ms);
                                        tag.toFrame = ReadMemory<ushort>(ms);
                                        tag.loopAnimDirection = ReadMemory<byte>(ms);
                                        tag.repeatCount = ReadMemory<ushort>(ms);
                                        IgnoreMemory<byte>(ms, 6);
                                        tag.tagColor = ReadMemory<PaletteColorRGB>(ms);
                                        IgnoreMemory<byte>(ms, 1);
                                        tag.tagName = ReadString(ms);

                                        tags.Add(tag);
                                    }
                                }
                                break;
                            case ChunkType.PaletteChunk:
                                {
                                    ignoreOldPalette = true;

                                    uint newPaletteSize = ReadMemory<uint>(ms);
                                    uint firstColorIndex = ReadMemory<uint>(ms);
                                    uint lastColorIndex = ReadMemory<uint>(ms);
                                    IgnoreMemory<byte>(ms, 8);

                                    int offset = (int)firstColorIndex;
                                    for (int p = 0, pmax = (int)(lastColorIndex - firstColorIndex + 1); p < pmax; ++p)
                                    {
                                        PaletteFlags paletteFlags = (PaletteFlags)ReadMemory<ushort>(ms);
                                        PaletteColorRGBA paletteColor = ReadMemory<PaletteColorRGBA>(ms);

                                        int idx = p + offset;
                                        while (palettes.Count <= idx)
                                        {
                                            palettes.Add(new()
                                            {
                                                color = default,
                                                name = "<unknown>"
                                            });
                                        }

                                        palettes[idx].color = paletteColor;

                                        if ((paletteFlags & PaletteFlags.HasName) == PaletteFlags.HasName)
                                        {
                                            palettes[idx].name = ReadString(ms);
                                        }
                                    }
                                }
                                break;
                            case ChunkType.UserDataChunk:
                                {
                                    UserData userData = new();
                                    UserDataFlags userDataFlags = (UserDataFlags)ReadMemory<uint>(ms);

                                    if ((userDataFlags & UserDataFlags.HasText) == UserDataFlags.HasText)
                                    {
                                        userData.text = ReadString(ms);
                                    }

                                    if ((userDataFlags & UserDataFlags.HasColor) == UserDataFlags.HasColor)
                                    {
                                        userData.color = ReadMemory<PixelRGBA>(ms);
                                    }

                                    if ((userDataFlags & UserDataFlags.HasProperties) == UserDataFlags.HasProperties)
                                    {
                                        uint sizeOfPropertyMaps = ReadMemory<uint>(ms);
                                        Assert(sizeOfPropertyMaps >= 8);

                                        uint numPropertyMaps = ReadMemory<uint>(ms);

                                        userData.propertyMaps = new UserData.PropertyMap[(int)numPropertyMaps];
                                        userData.propertyMapData = ReadMemory(ms, (int)(sizeOfPropertyMaps - 8));

                                        ProcessPropertyMap(userData);

                                        static void ProcessPropertyMap(UserData userData)
                                        {
                                            using MemoryStream ms = new(userData.propertyMapData, false);

                                            for (int p = 0, pmax = userData.propertyMaps.Length; p < pmax; ++p)
                                            {
                                                uint propertyMapKey = ReadMemory<uint>(ms);
                                                uint numProperties = ReadMemory<uint>(ms);

                                                UserData.PropertyMap propertyMap = (userData.propertyMaps[p] ??= new());
                                                propertyMap.key = propertyMapKey;

                                                for (int i = 0, imax = (int)numProperties; i < imax; ++i)
                                                {
                                                    string propertyName = ReadString(ms);
                                                    UserData.PropertyMap.PropertyType propertyType = (UserData.PropertyMap.PropertyType)ReadMemory<ushort>(ms);

                                                    UserData.PropertyMap.Property property = new()
                                                    {
                                                        type = propertyType,
                                                        offset = (int)ms.Position,
                                                    };

                                                    bool added = propertyMap.properties.TryAdd(propertyName, property);
                                                    Assert(added);

                                                    switch (propertyType)
                                                    {
                                                        case UserData.PropertyMap.PropertyType.Boolean:
                                                            IgnoreMemory<byte>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.Int8:
                                                            IgnoreMemory<byte>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.UInt8:
                                                            IgnoreMemory<byte>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.Int16:
                                                            IgnoreMemory<short>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.UInt16:
                                                            IgnoreMemory<ushort>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.Int32:
                                                            IgnoreMemory<int>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.UInt32:
                                                            IgnoreMemory<uint>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.Int64:
                                                            IgnoreMemory<long>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.UInt64:
                                                            IgnoreMemory<ulong>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.Fixed:
                                                            IgnoreMemory<FixedPoint>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.Float32:
                                                            IgnoreMemory<float>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.Float64:
                                                            IgnoreMemory<double>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.String:
                                                            ReadString(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.Point:
                                                            IgnoreMemory<Point>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.Size:
                                                            IgnoreMemory<Size>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.Rect:
                                                            IgnoreMemory<Rect>(ms);
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.Vector:
                                                            {
                                                                uint numElements = ReadMemory<uint>(ms);
                                                                ushort elementsType = ReadMemory<ushort>(ms);

                                                                // TODO: implemente
                                                                if (elementsType == 0)
                                                                {

                                                                }
                                                                else
                                                                {

                                                                }
                                                            }
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.PropertyMap:
                                                            {
                                                                // TODO: implement
                                                            }
                                                            break;
                                                        case UserData.PropertyMap.PropertyType.UUID:
                                                            IgnoreMemory<byte>(ms, 16);
                                                            break;
                                                        default:
                                                            InvalidCodePath($"Unknown Property Type {propertyType}");
                                                            break;
                                                    }

                                                }
                                            }
                                        }
                                    }

                                    switch (lastReadChunk)
                                    {
                                        case ChunkType.OldPaletteChunk1:
                                            Assert(palettes.Count > 0);
                                            (palettes[^1].userData ??= new()).Add(userData);
                                            break;
                                        case ChunkType.OldPaletteChunk2:
                                            Assert(palettes.Count > 0);
                                            (palettes[^1].userData ??= new()).Add(userData);
                                            break;
                                        case ChunkType.LayerChunk:
                                            Assert(layers.Count > 0);
                                            (layers[^1].userData ??= new()).Add(userData);
                                            break;
                                        case ChunkType.CelChunk:
                                            Assert(cels.Count > 0);
                                            (cels[^1].userData ??= new()).Add(userData);
                                            break;
                                        case ChunkType.CelExtraChunk:
                                            Assert(cels.Count > 0);
                                            (cels[^1].userData ??= new()).Add(userData);
                                            break;
                                        case ChunkType.ColorProfileChunk:
                                            InvalidCodePath("Should this track user data?");
                                            break;
                                        case ChunkType.ExternalFilesChunk:
                                            InvalidCodePath("Should this track user data?");
                                            break;
                                        case ChunkType.MaskChunk:
                                            InvalidCodePath("MaskChunk is deprecated");
                                            break;
                                        case ChunkType.PathChunk:
                                            InvalidCodePath("PathChunk is never used");
                                            break;
                                        case ChunkType.TagsChunk:
                                            Assert(tags.Count > 0);
                                            (tags[^1].userData ??= new()).Add(userData);
                                            break;
                                        case ChunkType.PaletteChunk:
                                            Assert(palettes.Count > 0);
                                            (palettes[^1].userData ??= new()).Add(userData);
                                            break;
                                        case ChunkType.UserDataChunk:
                                            InvalidCodePath("UserData should not be the Last Read Chunk");
                                            break;
                                        case ChunkType.SliceChunk:
                                            Assert(slices.Count > 0);
                                            (slices[^1].userData ??= new()).Add(userData);
                                            break;
                                        case ChunkType.TilesetChunk:
                                            Assert(tilesets.Count > 0);
                                            (tilesets[^1].userData ??= new()).Add(userData);
                                            break;
                                        default:
                                            InvalidCodePath($"Invalid Last Read Chunk {lastReadChunk}");
                                            break;
                                    }
                                }
                                break;
                            case ChunkType.SliceChunk:
                                {
                                    Slice slice = new();

                                    uint numSliceKeys = ReadMemory<uint>(ms);
                                    SliceFlags sliceFlags = (SliceFlags)ReadMemory<uint>(ms);
                                    IgnoreMemory<uint>(ms);
                                    slice.name = ReadString(ms);

                                    slice.sliceKeys = new Slice.Key[numSliceKeys];

                                    for (int s = 0, smax = (int)numSliceKeys; s < smax; ++s)
                                    {
                                        ref Slice.Key key = ref slice.sliceKeys[s];
                                        key.frameNumber = ReadMemory<uint>(ms);
                                        key.sliceXOrigin = ReadMemory<int>(ms);
                                        key.sliceYOrigin = ReadMemory<int>(ms);
                                        key.sliceWidth = ReadMemory<uint>(ms);
                                        key.sliceHeight = ReadMemory<uint>(ms);

                                        if ((sliceFlags & SliceFlags.NineSlice) == SliceFlags.NineSlice)
                                        {
                                            key.centerXPos = ReadMemory<int>(ms);
                                            key.centerYPos = ReadMemory<int>(ms);
                                            key.centerWidth = ReadMemory<uint>(ms);
                                            key.centerHeight = ReadMemory<uint>(ms);
                                        }

                                        if ((sliceFlags & SliceFlags.HasPivot) == SliceFlags.HasPivot)
                                        {
                                            key.pivotXPos = ReadMemory<int>(ms);
                                            key.pivotYPos = ReadMemory<int>(ms);
                                        }
                                    }

                                    slices.Add(slice);
                                }
                                break;
                            case ChunkType.TilesetChunk:
                                {
                                    Tileset tileset = new();

                                    tileset.tilesetID = ReadMemory<uint>(ms);
                                    tileset.flags = (TilesetFlags)ReadMemory<uint>(ms);
                                    tileset.numTiles = ReadMemory<uint>(ms);
                                    tileset.tileWidth = ReadMemory<ushort>(ms);
                                    tileset.tileHeight = ReadMemory<ushort>(ms);
                                    tileset.baseIndex = ReadMemory<short>(ms);
                                    IgnoreMemory<byte>(ms, 14);
                                    tileset.name = ReadString(ms);

                                    if ((tileset.flags & TilesetFlags.IncludeExternalFile) == TilesetFlags.IncludeExternalFile)
                                    {
                                        tileset.externalFileID = ReadMemory<uint>(ms);
                                        tileset.externalTilesetID = ReadMemory<uint>(ms);
                                    }

                                    if ((tileset.flags & TilesetFlags.IncludeTilesInFile) == TilesetFlags.IncludeTilesInFile)
                                    {
                                        uint compressedTilesetDataLength = ReadMemory<uint>(ms);

                                        using (ZLibStream compressedStream = new ZLibStream(ms, CompressionMode.Decompress, true))
                                        {
                                            tileset.tilesetData = DecompressMemory(compressedStream, (int)compressedTilesetDataLength, (int)(tileset.tileWidth * tileset.tileHeight * tileset.numTiles));
                                        }
                                    }

                                    tilesets.Add(tileset);
                                }
                                break;
                            default:
                                InvalidCodePath($"Unknown Chunk Type {chunkType}");
                                break;
                        }

                        lastReadChunk = chunkType;
                    }
                }
            }
            #endregion

            #region Convert Aseprite into CHR
            // convert into chr data
            StringBuilder sb = new();

            // write output file
            string chrOutputKey = $"{Path.GetFileName(asepriteFilename)}.sprite";
            outputChrData.Add(chrOutputKey, new()
            {
                chrRomAsm = sb.ToString(),
                chrCount = 0,
                is8x16 = false,
            });
            #endregion
        }
        #endregion

        static unsafe T ReadMemory<T>(Stream stream) where T : unmanaged
        {
            int size = Marshal.SizeOf<T>();
            Assert(stream.Position + size < stream.Length);
            Span<byte> buffer = stackalloc byte[size];
            int read = stream.Read(buffer);
            Assert(read == size);
            fixed (byte* p = buffer)
            {
                return *(T*)p;
            }
        }

        static string ReadString(Stream stream)
        {
            ushort length = ReadMemory<ushort>(stream);
            Span<byte> buffer = stackalloc byte[length];
            int read = stream.Read(buffer);
            Assert(read == length);
            return Encoding.UTF8.GetString(buffer);
        }

        static unsafe void ReadMemory(Stream stream, byte[] buffer, int length)
        {
            Assert(stream.Position + length < stream.Length);
            int read = stream.Read(buffer, 0, length);
            Assert(read == length);
        }

        static unsafe byte[] ReadMemory(Stream stream, int length)
        {
            Assert(stream.Position + length < stream.Length);
            byte[] buffer = new byte[length];
            int read = stream.Read(buffer, 0, length);
            Assert(read == length);
            return buffer;
        }

        static unsafe byte[] DecompressMemory(Stream stream, int compressedLength, int decompressedLength)
        {
            //Assert(stream.Position + compressedLength < stream.Length);
            byte[] buffer = new byte[decompressedLength];
            int read = stream.Read(buffer, 0, decompressedLength);
            Assert(read == decompressedLength);
            return buffer;
        }

        static unsafe void AssertReadMemoryEquals<T>(Stream stream, T value, IEqualityComparer<T> equalityComparer = null) where T : unmanaged
        {
            equalityComparer ??= EqualityComparer<T>.Default;
            Assert(equalityComparer.Equals(ReadMemory<T>(stream), value));
        }

        static void IgnoreMemory<T>(Stream ms, int length = 1) where T : unmanaged
        {
            int totalSize = Marshal.SizeOf<T>() * length;
            Assert(ms.Position + totalSize < ms.Length);
            ms.Position += totalSize;
        }

        #region Text File
        static void ConvertTextFile(string textFilename, Dictionary<string, ChrRomOutput> outputChrData, in ConvertOptions cmdOptions)
        {
            TryGetFileParamters(textFilename, "tr", out var remapCharsParameters);

            static bool ParseCSVElements(StreamReader sr, out List<string> values)
            {
                bool ok = true;

                values = new();
                StringBuilder sb = new();

                bool inQuote = false;
                bool inVerbatum = false;

                // trim initial line read
                string line = sr.ReadLine()?.TrimStart() ?? string.Empty;

                do
                {
                    for (int i = 0; i < line.Length; i++)
                    {
                        char c = line[i];

                        // skip comments
                        if (i == 0 && c == '#' && !inQuote)
                        {
                            break;
                        }

                        switch (c)
                        {
                            case '\\':
                                ++i;
                                if (i < line.Length)
                                {
                                    c = line[i];
                                    switch (c)
                                    {
                                        case '\\': sb.Append('\\'); break;
                                        case 'n': sb.Append('\n'); break;
                                        case 'r': sb.Append('\r'); break;
                                        case 't': sb.Append('\t'); break;
                                        case '\'': sb.Append('\''); break;
                                        case '"': sb.Append('"'); break;
                                        case '%': sb.Append('%'); break;
                                        default: InvalidCodePath($"Unkown switch command \\{c}"); break;
                                    }
                                }
                                break;

                            case '%':
                                inVerbatum = !inVerbatum;
                                sb.Append(c);
                                break;

                            case '"':
                                if (inQuote)
                                {
                                    if ((i + 1) < line.Length && line[i + 1] == '"')
                                    {
                                        if (inVerbatum)
                                        {
                                            sb.Append('"');
                                        }
                                        else
                                        {
                                            sb.Append("\\\"");
                                        }
                                        ++i;
                                    }
                                    else
                                    {
                                        inQuote = false;
                                    }
                                }
                                else
                                {
                                    inQuote = true;
                                }
                                break;

                            case ',':
                                if (inQuote || inVerbatum)
                                {
                                    sb.Append(c);
                                }
                                else
                                {
                                    values.Add(sb.ToString());
                                    sb.Clear();
                                }
                                break;

                            default:
                                sb.Append(c);
                                break;
                        }
                    }

                    // continue processing lines if we're still in a quote
                    if (inQuote)
                    {
                        // read next line, don't trim since there could be spaces needed
                        line = sr.ReadLine();

                        // if there are still lines, add a new line to the string buffer
                        if (line != null)
                        {
                            sb.Append("\\n");
                        }
                    }
                    else
                    {
                        line = null;
                    }
                } while (line != null);

                if (sb.Length > 0)
                {
                    values.Add(sb.ToString());
                    sb.Clear();
                }

                return values.Count > 0;
            }

            static bool IsVerbatumString(string str)
            {
                return str.StartsWith('%') && str.EndsWith('%');
            }

            FileStreamOptions fileOptions = new()
            {
                Mode = FileMode.Open,
                Access = FileAccess.Read
            };

            using var sr = new StreamReader(new FileStream(textFilename, fileOptions));

            ParseCSVElements(sr, out var headers);
            // language -> key -> value

            Dictionary<string, Dictionary<string, string>> textMap = new();

            // skip first header
            for (int i = 1; i < headers.Count; i++)
            {
                string lang = headers[i];
                textMap[lang] = new Dictionary<string, string>();
            }

            // parse each line into textMap
            while (!sr.EndOfStream)
            {
                if (ParseCSVElements(sr, out var values))
                {
                    string key = values[0];
                    for (int i = 1; i < values.Count; i++)
                    {
                        textMap[headers[i]][key] = values[i];
                    }
                }
            }

            HashSet<string> referencedKeys = new();

            // Resolve back references
            foreach (var langToMap in textMap)
            {
                foreach (var kv in langToMap.Value)
                {
                    string value = kv.Value;
                    referencedKeys.Clear();

                    bool changed = false;
                    int s;
                    while ((s = value.IndexOf("${")) >= 0)
                    {
                        int e = value.IndexOf('}', s);
                        if (e > s)
                        {
                            string refKey = value.Substring(s + 2, e - s - 2);

                            if (referencedKeys.Add(refKey))
                            {
                                if (langToMap.Value.TryGetValue(refKey, out string refValue))
                                {
                                    value = $"{value[..s]}{refValue}{value[(e + 1)..]}";

                                    changed = true;
                                }
                                else
                                {
                                    LogError($"Unable to find reference '{refKey}' in '{kv.Key}'");
                                    changed = false;
                                    break;
                                }
                            }
                            else
                            {
                                LogError($"Circular reference found in '{kv.Key}'");
                                changed = false;
                                break;
                            }
                        }
                    }

                    if (changed)
                    {
                        langToMap.Value[kv.Key] = value;
                    }
                }
            }

            StringBuilder sb = new();

            // Test for long strings
            const int kMaxStringWidth = 32;
            foreach (var langToMap in textMap)
            {
                foreach (var kv in langToMap.Value)
                {
                    string value = kv.Value;

                    if (value.Length > kMaxStringWidth && !IsVerbatumString(value))
                    {
                        // split on "new line" sequence
                        string[] lines = value.Split("\\n", StringSplitOptions.RemoveEmptyEntries);

                        bool allLinesShort = true;
                        foreach (string l in lines)
                        {
                            allLinesShort &= l.Length < kMaxStringWidth;
                        }

                        if (!allLinesShort)
                        {
                            LogError($"String too long [{langToMap.Key}]:[{kv.Key}] -> [{value}]");
                        }
                    }
                }
            }

            // replace single character escape sequence with full expanded sequence if ca65 supports escape sequences in strings
            const bool supportsEscapeSequences = true;
            if (supportsEscapeSequences)
            {
                foreach (var langToMap in textMap)
                {
                    foreach (var kv in langToMap.Value)
                    {
                        string value = kv.Value;
                        if (!IsVerbatumString(value))
                        {

                            value = value.Replace("\n", "\\n");
                            value = value.Replace("\t", "\\t");
                            value = value.Replace("\b", "\\b");
                            value = value.Replace("\"", "\\\"");
                            value = value.Replace("\'", "\\'");

                            langToMap.Value[kv.Key] = value;
                        }
                    }
                }
            }

            // remap characters if needed
            if (remapCharsParameters?.Count > 0)
            {
                foreach (var langToMap in textMap)
                {
                    foreach (var kv in langToMap.Value)
                    {
                        string value = kv.Value;

                        sb.Clear();
                        for (int i = 0; i < value.Length; i++)
                        {
                            char c = value[i];

                            string chrmapKey = $"charmap.{CharacterToDefineName(c)}";

                            string mappedChar = GetStringParameter(remapCharsParameters, chrmapKey, c.ToString());
                            sb.Append(mappedChar);
                        }

                        value = sb.ToString();

                        if (kv.Value != value)
                        {
                            langToMap.Value[kv.Key] = value;
                        }
                    }
                }
            }

            string filename = Path.GetFileNameWithoutExtension(textFilename);

            const string segment = "RODATA";

            // generate each language text files
            foreach (var langToMap in textMap)
            {
                string lang = langToMap.Key;
                string sFilename = Path.Combine(Path.GetDirectoryName(textFilename), cmdOptions.defaultGeneratedAssetFolder, "asm", $"{filename}.{lang}.s");

                sb.Clear();
                sb.AppendLine(";");
                sb.AppendLine($"; Text '{lang}'- Generated from {Path.GetFileName(textFilename)}");
                sb.AppendLine(";");
                sb.AppendLine();


                sb.AppendLine(";");
                sb.AppendLine("; Text Table");
                sb.AppendLine(";");
                sb.AppendLine();

                sb.AppendLine($".segment    \"{segment}\"");
                sb.AppendLine($"_text_table:");
                sb.AppendLine($".export _text_table");
                sb.AppendLine();

                foreach (var kv in langToMap.Value)
                {
                    sb.AppendLine($".addr   {kv.Key}");
                }
                sb.AppendLine();

                sb.AppendLine(";");
                sb.AppendLine("; Text");
                sb.AppendLine(";");
                sb.AppendLine();

                sb.AppendLine($".segment    \"{segment}\"");
                sb.AppendLine();

                foreach (var kv in langToMap.Value)
                {
                    string tr = $"{kv.Key}:";
                    if (IsVerbatumString(kv.Value))
                    {
                        sb.AppendLine($"{tr,-64} TR {kv.Value[1..^1]}");
                    }
                    else
                    {
                        sb.AppendLine($"{tr,-64} TR \"{kv.Value}\"");
                    }
                }

                File.WriteAllText(sFilename, sb.ToString(), UTF8);
                LogInfo($"Converted {textFilename} -> {sFilename}");
            }

            const string str_t = "str_t";
            const string uint8_t = "uint8_t";

            static StringBuilder Indent(StringBuilder sb, int indentLevel = 1, string tab = "    ")
            {
                for (int i = 0; i < indentLevel; ++i)
                {
                    sb.Append(tab);
                }

                return sb;
            }

            // generate each language header files
            foreach (var langToMap in textMap)
            {
                string lang = langToMap.Key.ToLower();
                string hFilename = Path.Combine(Path.GetDirectoryName(textFilename), cmdOptions.defaultGeneratedAssetFolder, "include", $"{filename}.{lang}.h");

                string defineGuard = Path.GetFileName(hFilename).ToUpper().Replace('.', '_');

                sb.Clear();
                sb.AppendLine("// ");
                sb.AppendLine($"// Generated from '{Path.GetFileName(textFilename)}'");
                sb.AppendLine("// ");
                sb.AppendLine();

                sb.AppendLine($"#ifndef {defineGuard}");
                sb.AppendLine($"#define {defineGuard}");
                sb.AppendLine();

                sb.AppendLine($"#define LANGUAGE_{lang.ToUpper()} \"{lang}\"");
                sb.AppendLine();

                sb.AppendLine("// string keys");
                sb.AppendLine("enum");
                sb.AppendLine("{");

                int i = 0;
                // output string keys
                // TODO: update this so there's not an implicit cap
                foreach (var kv in langToMap.Value)
                {
                    Indent(sb).Append($"{kv.Key,-64} = ({i++} << 1)").AppendLine(",");
                }

                sb.AppendLine("};");
                sb.AppendLine();

                sb.AppendLine("// string lengths");
                sb.AppendLine("enum");
                sb.AppendLine("{");

                // output string lengths
                foreach (var kv in langToMap.Value)
                {
                    string key = $"{kv.Key}_length";
                    Indent(sb).Append($"{key,-64} = {kv.Value.Length}").AppendLine(",");
                }

                sb.AppendLine("};");
                sb.AppendLine();

                sb.AppendLine("// string 'width' (max characters per line)");
                sb.AppendLine("enum");
                sb.AppendLine("{");

                // output string "width" (max characters per line)
                foreach (var kv in langToMap.Value)
                {
                    string[] lines = kv.Value.Split("\\n", StringSplitOptions.RemoveEmptyEntries);

                    int maxWidth = -1;
                    foreach (var line in lines)
                    {
                        maxWidth = Math.Max(maxWidth, line.Length);
                    }
                    Assert(maxWidth > 0);

                    Indent(sb).Append($"{$"{kv.Key}_width",-64} = {maxWidth}").AppendLine(",");
                }

                sb.AppendLine("};");
                sb.AppendLine();

                sb.AppendLine("// string 'height' (number of lines)");
                sb.AppendLine("enum");
                sb.AppendLine("{");

                // output string "width" (max characters per line)
                foreach (var kv in langToMap.Value)
                {
                    string[] lines = kv.Value.Split("\\n", StringSplitOptions.RemoveEmptyEntries);

                    int maxWidth = -1;
                    foreach (var line in lines)
                    {
                        maxWidth = Math.Max(maxWidth, line.Length);
                    }
                    Assert(maxWidth > 0);

                    Indent(sb).Append($"{$"{kv.Key}_height",-64} = {lines.Length}").AppendLine(",");
                }

                sb.AppendLine("};");
                sb.AppendLine();

                sb.AppendLine($"#endif // {defineGuard}");

                File.WriteAllText(hFilename, sb.ToString(), UTF8);
                LogInfo($"Converted {textFilename} -> {hFilename}");
            }
        }
        #endregion

        #region Layout File
        [StructLayout(LayoutKind.Sequential)]
        struct ChrRomLayout
        {
            public string chrRomKey;
            public int offset;
            public int length;
        }

        static void ConvertLayoutFile(string layoutFilename, Dictionary<string, ChrRomOutput> outputChrData, in ConvertOptions cmdOptions)
        {
            string chrSegment = Path.GetFileNameWithoutExtension(layoutFilename).ToUpperInvariant();
            Assert(Array.IndexOf(ChrRomSegments, chrSegment) >= 0, $"Unknown CHR ROM Layout segment {layoutFilename}");

            TryGetFileParamters(layoutFilename, string.Empty, out var layoutParameters);

            List<ChrRomLayout> chrRomLayouts = new();

            int offset = 0;
            foreach (var kv in layoutParameters)
            {
                bool ok = outputChrData.TryGetValue(kv.Key, out var chrRomOutput);
                if (ok)
                {
                    int chrOffset = GetIntParameter(layoutParameters, kv.Key, offset);
                    chrRomLayouts.Add(new()
                    {
                        chrRomKey = kv.Key,
                        offset = chrOffset,
                        length = chrRomOutput.chrCount
                    });

                    offset = Math.Max(offset + chrRomOutput.chrCount, chrOffset + chrRomOutput.chrCount);
                }
            }

            // sort chr rom by offsets
            chrRomLayouts.Sort((x, y) => x.offset.CompareTo(y.offset));

            StringBuilder sb = new();
            sb.AppendLine(";");
            sb.AppendLine($"; Generated from '{Path.GetFileName(layoutFilename)}'");
            sb.AppendLine(";");
            sb.AppendLine();

            sb.AppendLine(".pushseg");
            sb.AppendLine($".segment \"{chrSegment}\"");
            sb.AppendLine();

            for (offset = 0; offset <= 0xFF;)
            {
                int chrIndex = chrRomLayouts.FindIndex(x => offset >= x.offset && offset < (x.offset + x.length));
                if (chrIndex >= 0)
                {
                    var chrRom = chrRomLayouts[chrIndex];
                    bool ok = outputChrData.TryGetValue(chrRom.chrRomKey, out var chrRomOutput);
                    Assert(ok, $"Unable to find CHR ROM Output {chrRom.chrRomKey}");
                    sb.Append(chrRomOutput.chrRomAsm);

                    offset += chrRom.length;
                    Assert(offset < 0xFF, $"{chrSegment} overrun {offset}");
                }
                else
                {
                    sb.AppendLine("; Empty");
                    sb.AppendLine(".byte 0, 0, 0, 0, 0, 0, 0, 0");
                    sb.AppendLine(".byte 0, 0, 0, 0, 0, 0, 0, 0");
                    sb.AppendLine();

                    ++offset;
                }
            }

            sb.AppendLine(".popseg");

            // write output file
            string sFilename = Path.Combine(Path.GetDirectoryName(layoutFilename), cmdOptions.defaultGeneratedAssetFolder, "asm", $"{chrSegment.ToLowerInvariant()}.s");
            File.WriteAllText(sFilename, sb.ToString(), UTF8);
            LogInfo($"Converted {layoutFilename} -> {sFilename}");
        }
        #endregion

        static UTF8Encoding UTF8 = new UTF8Encoding(false);

        static string[] ChrRomSegments = {
            "CHR_00",
            "CHR_01",
            "CHR_02",
            "CHR_03",
            "CHR_04",
            "CHR_05",
            "CHR_06",
            "CHR_07",
            "CHR_08",
            "CHR_09",
            "CHR_0A",
            "CHR_0B",
            "CHR_0C",
            "CHR_0D",
            "CHR_0E",
            "CHR_0F",
            "CHR_10",
            "CHR_11",
            "CHR_12",
            "CHR_13",
            "CHR_14",
            "CHR_15",
            "CHR_16",
            "CHR_17",
            "CHR_18",
            "CHR_19",
            "CHR_1A",
            "CHR_1B",
            "CHR_1C",
            "CHR_1D",
            "CHR_1E",
            "CHR_1F",
        };

        private const int LibraryVersion = 2;

        class CompileTask
        {
            public string inputFileName;
            public ConvertFunc convertFunc;
            public int order;
        }

        struct SupportedFormat
        {
            public ConvertFunc convertFunc;
            public int order;
        }

        static int ExitCode = 0;

        static int Main(string[] args)
        {
#if DEBUG
            if (Debugger.IsAttached)
            {
                args = new[] { @"assets\" };
            }
#endif
            ExitCode = 0;

            ConvertOptions options = new()
            {
                defaultGeneratedAssetFolder = "generated"
            };

            Dictionary<string, SupportedFormat> formatMap = new() {
                { ".png", new() { convertFunc = ConvertImageFile, order = 0 } },
                { ".bmp", new() { convertFunc = ConvertImageFile, order = 0 } },
                { ".jpg", new() { convertFunc = ConvertImageFile, order = 0 } },
                { ".jpeg", new() { convertFunc = ConvertImageFile, order = 0 } },
                { ".gif", new() { convertFunc = ConvertImageFile, order = 0 } },
                { ".tga", new() { convertFunc = ConvertImageFile, order = 0 } },

                { ".txt", new() { convertFunc = ConvertTextFile, order = 0 } },
                { ".csv", new() { convertFunc = ConvertTextFile, order = 0 } },

                { ".layout", new() {convertFunc = ConvertLayoutFile, order = int.MaxValue } },

                //{ ".aseprite", new(){ convertFunc = ConverteAsepriteFile, order = 0 } },
            };

            List<CompileTask> compilerTasks = new();

            // TODO: force reimport turned on because of new chr rom layout
            //   dependency thing needed to make sure that all sprites are generated in chr rom when one changes to make sure it's generated correctly
            bool forceReimport = true;

#if DEBUG
            forceReimport |= Debugger.IsAttached;
#endif
            string cwd = Directory.GetCurrentDirectory();
            LogInfo($"{nameof(cwd)} {cwd}");

            // process commandline args
            for (int i = 0; i < args.Length; ++i)
            {
                string arg = args[i];
                if (arg.StartsWith('-'))
                {
                    switch (arg.ToLowerInvariant())
                    {
                        case "-h":
                        case "--help":
                            return ExitCode;

                        case "-f":
                        case "--force-reimport":
                            forceReimport = true;
                            break;

                        case "-g":
                        case "--generated-dir":
                            ++i;
                            if (i < args.Length)
                            {
                                options.defaultGeneratedAssetFolder = args[i];
                            }
                            break;
                    }
                }
                else
                {
                    string argPath = Path.Combine(cwd, arg);
                    LogInfo($"{nameof(argPath)} {argPath}");
                    if (Directory.Exists(argPath))
                    {
                        LogInfo($"Process directory {argPath}...");
                        if (!TryGetFileParamters(Path.Combine(argPath, ""), "library", out var libraryParameters) || forceReimport)
                        {
                            libraryParameters = new Dictionary<string, string>();
                        }

                        // if there's a version mismatch, perform reimport
                        if (LibraryVersion != GetIntParameter(libraryParameters, ".version", 0))
                        {
                            libraryParameters.Clear();
                        }

                        SetParameter(libraryParameters, ".version", LibraryVersion);

                        bool updateLibrary = false;

                        var files = Directory.GetFiles(argPath);
                        foreach (var file in files)
                        {
                            var ext = Path.GetExtension(file);
                            if (formatMap.TryGetValue(ext, out var supportedFormat))
                            {
                                string modtimeKey = $"{Path.GetFileName(file)}.modtime";
                                long modtime = GetLongParameter(libraryParameters, modtimeKey);
                                long fileModtime = File.GetLastWriteTimeUtc(file).Ticks;

                                if (modtime != fileModtime)
                                {
                                    SetParameter(libraryParameters, modtimeKey, fileModtime);
                                    updateLibrary = true;

                                    compilerTasks.Add(new() { inputFileName = file, convertFunc = supportedFormat.convertFunc, order = supportedFormat.order });
                                }
                            }
                        }

                        if (updateLibrary)
                        {
                            WriteParametersToFile(libraryParameters, Path.Combine(argPath, ".library"));
                        }
                    }
                    else if (File.Exists(argPath))
                    {
                        LogInfo($"Process file {argPath}...");
                        if (!TryGetFileParamters(Path.Combine(Path.GetDirectoryName(argPath), ""), "library", out var libraryParameters) || forceReimport)
                        {
                            libraryParameters = new Dictionary<string, string>();
                        }

                        // if there's a version mismatch, perform reimport
                        if (LibraryVersion != GetIntParameter(libraryParameters, ".version", 0))
                        {
                            libraryParameters.Clear();
                        }

                        SetParameter(libraryParameters, ".version", LibraryVersion);

                        bool updateLibrary = false;

                        var ext = Path.GetExtension(argPath);
                        if (formatMap.TryGetValue(ext, out var supportedFormat))
                        {
                            string modtimeKey = $"{Path.GetFileName(argPath)}.modtime";
                            long modtime = GetLongParameter(libraryParameters, modtimeKey);
                            long fileModtime = File.GetLastWriteTimeUtc(argPath).Ticks;

                            if (modtime != fileModtime)
                            {
                                SetParameter(libraryParameters, modtimeKey, fileModtime);
                                updateLibrary = true;

                                compilerTasks.Add(new() { inputFileName = argPath, convertFunc = supportedFormat.convertFunc, order = supportedFormat.order });
                            }
                        }

                        if (updateLibrary)
                        {
                            WriteParametersToFile(libraryParameters, Path.Combine(Path.GetDirectoryName(argPath), ".library"));
                        }
                    }
                }
            }

            // create output generated folders
            if (compilerTasks.Count > 0)
            {
                HashSet<string> outputGenDirs = new();
                foreach (var task in compilerTasks)
                {
                    string outGenDir = Path.Combine(Path.GetDirectoryName(compilerTasks[0].inputFileName), options.defaultGeneratedAssetFolder);
                    outputGenDirs.Add(outGenDir);
                }

                foreach (var dir in outputGenDirs)
                {
                    LogInfo($"Output Gen: {dir}");
                    Directory.CreateDirectory(dir);
                    Directory.CreateDirectory(Path.Combine(dir, "asm"));
                    Directory.CreateDirectory(Path.Combine(dir, "include"));
                }
            }
            else
            {
                LogWarning($"No compiler tasks generated: {string.Join(',', args)}");
            }

            // sort compiler tasks
            compilerTasks.Sort((x, y) => x.order.CompareTo(y.order));

            // process compiler tasks
            Dictionary<string, ChrRomOutput> outputChrData = new();
            foreach (var task in compilerTasks)
            {
                task.convertFunc(task.inputFileName, outputChrData, options);
            }

            return ExitCode;
        }
    }
}

#pragma warning restore CA1416

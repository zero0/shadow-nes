using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
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
            public string defaultAssetLayoutFileName;
            public string defaultAssetLayoutFileExt;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct UniqueTileEntry
        {
            public int index;
            public int attr;
        }

        [Conditional("DEBUG")]
        static void LogInfo(string msg)
        {
            Console.WriteLine(msg);
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

        static void LogError(string msg)
        {
            Console.Error.WriteLine(msg);
        }

        static bool Assert(bool condition, string msg = null, [CallerFilePath] string filepath = "", [CallerMemberName] string memberName = "", [CallerLineNumber] int line = 0)
        {
            if (!condition)
            {
                Console.Error.WriteLine($"Assert failed {filepath}:{line}{(msg == null ? "" : $": {msg}")}");
            }
            return !condition;
        }

        static bool TryGetFileParamters(string srcFilename, string parameterFileExt, out Dictionary<string, string> parameters)
        {
            parameters = null;

            string spriteFilename = $"{srcFilename}.{parameterFileExt}";
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
            int value = parameters?.TryGetValue(key, out string strValue) ?? false ? int.TryParse(strValue, out int intValue) ? intValue : defaultValue : defaultValue;
            return value;
        }

        static long GetLongParameter(Dictionary<string, string> parameters, string key, long defaultValue = 0)
        {
            long value = parameters?.TryGetValue(key, out string strValue) ?? false ? long.TryParse(strValue, out long longValue) ? longValue : defaultValue : defaultValue;
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

        delegate void ConvertFunc(string inputFilename, in ConvertOptions convertOptions);

        const string uint8Type = "uint8_t";

        static void ConvertImageFile(string imageFilename, in ConvertOptions cmdOptions)
        {
            ConvertOptions options = cmdOptions;

            TryGetFileParamters(options.defaultAssetLayoutFileName, options.defaultAssetLayoutFileExt, out var assetLayoutParameters);

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

            string assetLayoutSpriteKey = $"{imageFilename}.sprite";
            string defaultChrSegment = GetStringParameter(assetLayoutParameters, $"{assetLayoutSpriteKey}.chr-rom", ChrRomSegments[0]);

            // generate text file
            const string rodataSegment = "RODATA";
            string chrRomSegment = GetStringParameter(spriteParameters, "chr-rom", defaultChrSegment);
            int chrRomSegmentIndex = Array.IndexOf(ChrRomSegments, chrRomSegment);
            Assert(chrRomSegmentIndex >= 0, $"Invalid CHR ROM Segment: {chrRomSegment}");

            StringBuilder sb = new StringBuilder();
            StringBuilder byte0 = new StringBuilder();
            StringBuilder byte1 = new StringBuilder();

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
            sb.AppendLine();
            sb.AppendLine($".segment \"{chrRomSegment}\"");
            sb.AppendLine();
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

            // output
            {
                string sFilename = Path.Combine(Path.GetDirectoryName(imageFilename), filename) + ".s";
                File.WriteAllText(sFilename, sb.ToString(), UTF8);
                LogInfo($"Converted {imageFilename} -> {sFilename}");
            }

            // Generate sprite header file
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

                sb.AppendLine($"#define {$"{exportName.ToUpper()}_CHR_ROM",-40} ({uint8Type})({chrRomSegmentIndex})");

                for (int i = 0; i < allUniqueTiles.Count; i++)
                {
                    UniqueTileEntry uniqueTileEntry = allUniqueTiles[i];
                    TileEntry tileEntry = allTiles[uniqueTileEntry.index];

                    sb.AppendLine($"#define {$"{exportName.ToUpper()}_{tileEntry.x}x{tileEntry.y}_CHR_ROM",-40} ({uint8Type})({chrRomSegmentIndex})");
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
                                        default: Assert(false, $"Invalid escape character {c}"); break;
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
                        string sFilename = Path.Combine(Path.GetDirectoryName(imageFilename), filename) + ".font.s";
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
                                sb.AppendLine($"#define {$"FONT_CHAR_{defineName.ToUpperInvariant()}",-40} (${uint8Type})({charOffset + index})");
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
                                        default: Assert(false, $"Invalid escape character {c}"); break;
                                    }
                                }

                                string defineName = CharacterToDefineName(c);

                                if (!string.IsNullOrEmpty(defineName))
                                {
                                    sb.AppendLine($"#define {$"FONT_CHAR_{defineName.ToUpperInvariant()}",-40} (${uint8Type})({charOffset + index})");
                                }
                            }
                        }

                        sb.AppendLine();
                        sb.AppendLine($"#endif // {defineGuard}");

                        // write font file
                        string hFilename = Path.Combine(Path.GetDirectoryName(imageFilename), filename) + ".font.h";
                        File.WriteAllText(hFilename, sb.ToString(), UTF8);
                        LogInfo($"Converted {imageFilename} -> {hFilename}");
                    }
                }
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
                        defineName = $"upper_{c}";
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
                            Assert(false, $"Unable to find name for character '{c}'");
                            break;
                    }
                }

                return defineName;
            }
        }

        static void ConvertTextFile(string textFilename, in ConvertOptions cmdOptions)
        {
            TryGetFileParamters(textFilename, "tr", out var parameters);

            static bool ParseElements(string line, out List<string> values)
            {
                bool ok = true;

                values = new();
                StringBuilder sb = new();

                bool inQuote = false;
                bool inVerbatum = false;
                for (int i = 0; i < line.Length; i++)
                {
                    char c = line[i];
                    switch (c)
                    {
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
                            if (inQuote)
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

                if (sb.Length > 0)
                {
                    values.Add(sb.ToString());
                    sb.Clear();
                }

                return ok;
            }

            FileStreamOptions fileOptions = new()
            {
                Mode = FileMode.Open,
                Access = FileAccess.Read
            };

            using var fr = new StreamReader(new FileStream(textFilename, fileOptions));

            string line = fr.ReadLine();

            ParseElements(line, out var headers);

            // language -> key -> value
            Dictionary<string, Dictionary<string, string>> textMap = new();

            // skip first header
            for (int i = 1; i < headers.Count; i++)
            {
                string lang = headers[i];
                textMap[lang] = new Dictionary<string, string>();
            }

            // parse each line into textMap
            while ((line = fr.ReadLine()) != null)
            {
                if (line.Length == 0 || line.TrimStart().StartsWith('#'))
                {
                    continue;
                }
                else if (ParseElements(line, out var values))
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
                                    Assert(false, $"Unable to find reference '{refKey}' in '{kv.Key}'");
                                    changed = false;
                                    break;
                                }
                            }
                            else
                            {
                                Assert(false, $"Circular reference found in '{kv.Key}'");
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

            // remap characters
            foreach (var langToMap in textMap)
            {
                foreach (var kv in langToMap.Value)
                {
                    string value = kv.Value;
                    bool changed = false;

                    sb.Clear();
                    for (int i = 0; i < value.Length; i++)
                    {
                        char c = value[i];

                        string chrmapKey = $"charmap.{c}";

                        char mappedChar = GetCharParameter(parameters, chrmapKey, c);
                        sb.Append(mappedChar);

                        changed |= mappedChar != c;
                    }

                    if (changed)
                    {
                        langToMap.Value[kv.Key] = sb.ToString();
                    }
                }
            }

            string filename = Path.GetFileNameWithoutExtension(textFilename);

            const string segment = "RODATA";

            // generate each language text files
            foreach (var langToMap in textMap)
            {
                string lang = langToMap.Key;
                string sFilename = $"{Path.Combine(Path.GetDirectoryName(textFilename), filename)}.{lang}.s";

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
                    if (kv.Value.StartsWith('%') && kv.Value.EndsWith('%'))
                    {
                        sb.AppendLine($"{tr,-32} TR {kv.Value[1..^1]}");
                    }
                    else
                    {
                        sb.AppendLine($"{tr,-32} TR \"{kv.Value}\"");
                    }
                }

                File.WriteAllText(sFilename, sb.ToString(), UTF8);
                LogInfo($"Converted {textFilename} -> {sFilename}");
            }

            // generate each language header files
            foreach (var langToMap in textMap)
            {
                string lang = langToMap.Key;
                string hFilename = $"{Path.Combine(Path.GetDirectoryName(textFilename), filename)}.{lang}.h";

                string defineGuard = Path.GetFileName(hFilename).ToUpper().Replace('.', '_');

                sb.Clear();
                sb.AppendLine("// ");
                sb.AppendLine($"// Generated from '{Path.GetFileName(textFilename)}'");
                sb.AppendLine("// ");
                sb.AppendLine();

                sb.AppendLine($"#ifndef {defineGuard}");
                sb.AppendLine($"#define {defineGuard}");
                sb.AppendLine();

                sb.AppendLine($"#define LANGUAGE \"{lang}\"");
                sb.AppendLine();

                int i = 0;
                foreach (var kv in langToMap.Value)
                {
                    sb.AppendLine($"#define {kv.Key,-32} (str_t)({i++} << 1)");
                }

                sb.AppendLine();
                sb.AppendLine($"#endif // {defineGuard}");

                File.WriteAllText(hFilename, sb.ToString(), UTF8);
                LogInfo($"Converted {textFilename} -> {hFilename}");
            }
        }

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

        static void Main(string[] args)
        {
            ConvertOptions options = new()
            {
                defaultAssetLayoutFileName = "assets",
                defaultAssetLayoutFileExt = "layout"
            };

            Dictionary<string, ConvertFunc> formatMap = new() {
                { ".png", ConvertImageFile },
                { ".bmp", ConvertImageFile },
                { ".jpg", ConvertImageFile },
                { ".jpeg", ConvertImageFile },
                { ".gif", ConvertImageFile },
                { ".tga", ConvertImageFile },

                { ".txt", ConvertTextFile },
            };

            bool forceReimport = false;

            for (int i = 0; i < args.Length; i++)
            {
                string arg = args[i];
                if (arg.StartsWith("-"))
                {
                    switch (arg.ToLowerInvariant())
                    {
                        case "-h":
                        case "--help":
                            return;

                        case "-f":
                        case "--force-reimport":
                            forceReimport = true;
                            break;
                    }
                }
                else
                {
                    if (Directory.Exists(arg))
                    {
                        if (!TryGetFileParamters(Path.Combine(arg, ""), "library", out var libraryParameters) || forceReimport)
                        {
                            libraryParameters = new Dictionary<string, string>();
                        }

                        bool updateLibrary = false;

                        var files = Directory.GetFiles(arg);
                        foreach (var file in files)
                        {
                            var ext = Path.GetExtension(file);
                            if (formatMap.TryGetValue(ext, out var func))
                            {
                                string modtimeKey = $"{Path.GetFileName(file)}.modtime";
                                long modtime = GetLongParameter(libraryParameters, modtimeKey);
                                long fileModtime = File.GetLastWriteTimeUtc(file).Ticks;

                                if (modtime != fileModtime)
                                {
                                    SetParameter(libraryParameters, modtimeKey, fileModtime);
                                    updateLibrary = true;

                                    func(file, options);
                                }
                            }
                        }

                        if (updateLibrary)
                        {
                            WriteParametersToFile(libraryParameters, Path.Combine(arg, ".library"));
                        }
                    }
                    else if (File.Exists(arg))
                    {
                        if (!TryGetFileParamters(Path.Combine(Path.GetDirectoryName(arg), ""), "library", out var libraryParameters) || forceReimport)
                        {
                            libraryParameters = new Dictionary<string, string>();
                        }

                        bool updateLibrary = false;

                        var ext = Path.GetExtension(arg);
                        if (formatMap.TryGetValue(ext, out var func))
                        {
                            string modtimeKey = $"{Path.GetFileName(arg)}.modtime";
                            long modtime = GetLongParameter(libraryParameters, modtimeKey);
                            long fileModtime = File.GetLastWriteTimeUtc(arg).Ticks;

                            if (modtime != fileModtime)
                            {
                                SetParameter(libraryParameters, modtimeKey, fileModtime);
                                updateLibrary = true;

                                func(arg, options);
                            }
                        }

                        if (updateLibrary)
                        {
                            WriteParametersToFile(libraryParameters, Path.Combine(Path.GetDirectoryName(arg), ".library"));
                        }
                    }
                }
            }
        }
    }
}

#pragma warning restore CA1416

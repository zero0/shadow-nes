using System;
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

        [Conditional("DEBUG")]
        static void LogInfo(string msg)
        {
            Console.WriteLine(msg);
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

        static void ConvertImageFile(string imageFilename)
        {
            string filename = Path.GetFileNameWithoutExtension(imageFilename);
            string sFilename = Path.Combine(Path.GetDirectoryName(imageFilename), filename) + ".s";

            FileStreamOptions options = new()
            {
                Mode = FileMode.Open,
                Access = FileAccess.Read
            };

            using var bitmapImage = new Bitmap(new FileStream(imageFilename, options));

            Assert(bitmapImage.Width >= 8, $"Image too small, width: {bitmapImage.Width} requried at least 8px");
            Assert(bitmapImage.Height >= 8, $"Image too small, height: {bitmapImage.Height} requried at least 8px");

            StringBuilder sb = new StringBuilder();
            sb.AppendLine(";");
            sb.AppendLine($"; Generated from {imageFilename}");
            sb.AppendLine(";");

            sb.AppendLine();

            StringBuilder byte0 = new StringBuilder();
            StringBuilder byte1 = new StringBuilder();

            //// read palette from sprite 0,0
            //SpritePalette[] palette = new SpritePalette[2 * 8];
            //
            //for (int y = 0, i = 0; y < 8; ++y)
            //{
            //    for (int x = 0; x < 8; x += 4)
            //    {
            //        palette[i].c0 = pngImage.GetPixel(x + 0, y);
            //        palette[i].c1 = pngImage.GetPixel(x + 1, y);
            //        palette[i].c2 = pngImage.GetPixel(x + 2, y);
            //        palette[i].c3 = pngImage.GetPixel(x + 3, y);
            //
            //        ++i;
            //    }
            //}

            // read first 4 pixels as palette
            SpritePalette[] palettes = new SpritePalette[1];
            palettes[0].c0 = bitmapImage.GetPixel(0, 0);
            palettes[0].c1 = bitmapImage.GetPixel(1, 0);
            palettes[0].c2 = bitmapImage.GetPixel(2, 0);
            palettes[0].c3 = bitmapImage.GetPixel(3, 0);

            sb.AppendLine("; Palette");
            sb.AppendLine($";  0: [{palettes[0].c0.R:D3}, {palettes[0].c0.G}, {palettes[0].c0.B}]");
            sb.AppendLine($";  1: [{palettes[0].c1.R:D3}, {palettes[0].c1.G}, {palettes[0].c1.B}]");
            sb.AppendLine($";  2: [{palettes[0].c2.R:D3}, {palettes[0].c2.G}, {palettes[0].c2.B}]");
            sb.AppendLine($";  3: [{palettes[0].c3.R:D3}, {palettes[0].c3.G}, {palettes[0].c3.B}]");
            sb.AppendLine();

            sb.AppendLine(".segment \"CHARS\"");
            sb.AppendLine();

            static void GetPaletteAndIndex(Color color, SpritePalette[] palettes, out int palette, out int index)
            {
                palette = -1;
                index = -1;

                for (int p = 0; p < palettes.Length; p++)
                {
                    SpritePalette spritePalette = palettes[p];
                    for (int i = 0; i < 4; ++i)
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

            // read sprites (starging from 8,0 to leave room for palette info)
            for (int y = 0, ymax = bitmapImage.Height; y < ymax; y += 8)
            {
                for (int x = y == 0 ? 8 : 0, xmax = bitmapImage.Width; x < xmax; x += 8)
                {
                    byte0.Clear();
                    byte1.Clear();

                    for (int py = 0; py < 8; ++py)
                    {
                        byte0.Append(".byte %");
                        byte1.Append(".byte %");

                        for (int px = 0; px < 8; ++px)
                        {
                            int vx = x + px;
                            int vy = y + py;
                            Assert(vx < bitmapImage.Width);
                            Assert(vy < bitmapImage.Height);

                            Color pixel = bitmapImage.GetPixel(vx, vy);

                            GetPaletteAndIndex(pixel, palettes, out var _, out int pixelIndex);

                            byte0.Append((pixelIndex & 0x1) == 0x1 ? "1" : "0");
                            byte1.Append((pixelIndex & 0x2) == 0x2 ? "1" : "0");
                        }

                        byte0.AppendLine();
                        byte1.AppendLine();
                    }

                    sb.AppendLine($"; {x},{y}");
                    sb.Append(byte0.ToString());
                    sb.AppendLine();
                    sb.Append(byte1.ToString());
                    sb.AppendLine();
                    sb.AppendLine();
                }
            }

            // output
            File.WriteAllText(sFilename, sb.ToString(), new UTF8Encoding(false));
            LogInfo($"Converted {imageFilename} -> {sFilename}");
        }

        static void Main(string[] args)
        {
            string[] supportedFormats = { ".png", ".bmp", ".jpg", ".jpeg", ".gif" };
            foreach (string filepath in args)
            {
                if (Directory.Exists(filepath))
                {
                    var files = Directory.GetFiles(filepath);
                    foreach (var file in files)
                    {
                        if (Array.IndexOf(supportedFormats, Path.GetExtension(file)) >= 0)
                        {
                            ConvertImageFile(file);
                        }
                    }
                }
                else if (File.Exists(filepath))
                {
                    ConvertImageFile(filepath);
                }
            }
        }
    }
}

#pragma warning restore CA1416

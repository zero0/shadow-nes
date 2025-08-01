using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Drawing;
using System.IO;
using System.IO.Compression;
using System.Reflection.Metadata;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;
using Parameters = System.Collections.Generic.Dictionary<string, string>;

#if false
using ImageBitmap = System.Drawing.Bitmap;
#else
using ImageBitmap = img2chr.Program.InternalBitmap;
#endif

namespace img2chr
{
    class Program
    {
        #region InternalBitmap
        internal class InternalBitmap : IDisposable
        {
            private enum ImageFormat
            {
                Unknown,
                PNG,
                TGA,
                BMP,
            }

            private int _width = 0;
            private int _height = 0;
            private List<Color> _pixels = new();

            public int Width => _width;

            public int Height => _height;

            public InternalBitmap(Stream stream)
            {
                long initPos = stream.Position;
                ImageFormat imageFormat = DetermineImageFormat(stream);
                switch (imageFormat)
                {
                    case ImageFormat.PNG:
                        stream.Position = initPos;
                        ParsePNG(stream, _pixels, ref _width, ref _height);
                        break;
                    case ImageFormat.TGA:
                        stream.Position = initPos;
                        ParseTGA(stream, _pixels, ref _width, ref _height);
                        break;
                    default:
                        throw new Exception("Unknown image format");
                }
            }

            public Color GetPixel(int x, int y)
            {
                Assert(x >= 0 && x < _width);
                Assert(y >= 0 && y < _height);

                int px = x < 0 ? 0 : x > _width ? _width - 1 : x;
                int py = y < 0 ? 0 : y > _height ? _height - 1 : y;

                int offset = py * _width + px;
                Assert(offset < _pixels.Count);

                int poffset = offset < 0 ? 0 : offset > _pixels.Count ? _pixels.Count - 1 : offset;

                return _pixels[poffset];
            }

            public void Dispose()
            {
                _pixels.Clear();
                _pixels = null;
            }

            private static ImageFormat DetermineImageFormat(Stream stream)
            {
                return ImageFormat.PNG;
            }

            #region PNG Parseing

            private struct ColorRGB8
            {
                public byte r, g, b;
            }

            private enum PNGColorType
            {
                Greyscale = 0,
                TrueColor = 2,
                IndexColor = 3,
                GreyscaleWithAlpha = 4,
                TrueColorWithAlpha = 6,
            }

            private enum PNGCompressionMethod
            {
                Deflate = 0,
            }

            private enum PNGFilterMethod
            {
                Adaptive = 0,
            }

            private enum PNGFilterMethodAdaptiveType
            {
                None = 0,
                Sub = 1,
                Up = 2,
                Average = 3,
                Paeth = 4,
            }

            private enum PNGInterlaceMethod
            {
                None = 0,
                Adam7 = 1,
            }

            private struct PNGContext : IDisposable
            {
                public uint width;
                public uint height;
                public byte bitDepth;

                public PNGColorType colorType;

                public PNGCompressionMethod compressionMethod;
                public PNGFilterMethod filterMethod;
                public PNGInterlaceMethod interlaceMethod;

                public MemoryStream compressedData;

                public byte[] rawData;

                public float? gamma;

                public ColorRGB8[] palette;

                public byte[] paletteAlpha;

                public Dictionary<string, string> text;

                public byte grayscaleSignificantBits;
                public byte redSignificantBits;
                public byte greenSignificantBits;
                public byte blueSignificantBits;
                public byte alphaSignificantBits;

                public void Decompress()
                {
                    compressedData.Position = 0;

                    using var decompressor = new ZLibStream(compressedData, CompressionMode.Decompress);
                    using var decompressedData = new MemoryStream();
                    decompressor.CopyTo(decompressedData);

                    rawData = decompressedData.ToArray();

                    LogInfo($"Decompress {compressedData.Length} -> {rawData.Length}");

                    //File.WriteAllBytes($"dump-{DateTime.UtcNow.Ticks}.dat", rawData);
                }

                public Color GetColorAt(int x, int y)
                {
                    int r, g, b, a;
                    r = 0;
                    g = 0;
                    b = 0;
                    a = 255;

                    static int GetRawDataAt(int x, int y, in PNGContext context)
                    {
                        x = Math.Clamp(x, 0, (int)context.width);
                        y = Math.Clamp(y, 0, (int)context.height);

                        int index = ((y * (int)context.width + x) * context.bitDepth + y * 8 + 8) / 8;
                        int paletteIndex = -1;
                        switch (context.bitDepth)
                        {
                            case 4:
                                int idx = context.rawData[index];
                                paletteIndex = idx << (4 * (x % 2));
                                paletteIndex = (0x0F & (paletteIndex >> 4));
                                break;
                            case 8:
                                paletteIndex = context.rawData[index];
                                break;
                            case 16:
                                paletteIndex = BitConverter.ToInt16(context.rawData, index);
                                break;
                            case 24:
                                paletteIndex = (int)(BitConverter.ToUInt32(context.rawData, index) >> 4);
                                break;
                            case 32:
                                paletteIndex = BitConverter.ToInt32(context.rawData, index);
                                break;
                            default:
                                InvalidCodePath();
                                break;
                        }
                        return paletteIndex;
                    }

                    switch (colorType)
                    {
                        case PNGColorType.Greyscale:
                            InvalidCodePath("Implement Greyscale");
                            break;

                        case PNGColorType.TrueColor:
                            InvalidCodePath("Implement TrueColor");
                            break;

                        case PNGColorType.IndexColor:
                            //if (index < rawData?.Length)
                            {
                                int rawDataX = GetRawDataAt(x, y, in this);
                                int rawDataA = GetRawDataAt(x - 1, y, in this);
                                int rawDataB = GetRawDataAt(x, y - 1, in this);
                                int rawDataC = GetRawDataAt(x - 1, y - 1, in this);
                                int paletteIndex = -1;

                                if (filterMethod == PNGFilterMethod.Adaptive)
                                {
                                    int scanline = (y * ((int)width) * bitDepth + y * 8) / 8;
                                    byte filterMode = rawData[scanline];

                                    switch ((PNGFilterMethodAdaptiveType)filterMode)
                                    {
                                        case PNGFilterMethodAdaptiveType.None:
                                            paletteIndex = rawDataX;
                                            break;
                                        case PNGFilterMethodAdaptiveType.Sub:
                                            paletteIndex = rawDataX + rawDataA;
                                            break;
                                        case PNGFilterMethodAdaptiveType.Up:
                                            paletteIndex = rawDataX + rawDataB;
                                            break;
                                        case PNGFilterMethodAdaptiveType.Average:
                                            paletteIndex = rawDataX + ((rawDataA + rawDataB) / 2);
                                            break;
                                        case PNGFilterMethodAdaptiveType.Paeth:
                                            paletteIndex = rawDataX + PaethPredictor(rawDataA, rawDataB, rawDataC);
                                            break;
                                        default:
                                            InvalidCodePath($"scanline {scanline} (x {x} y {y} w {width} h {height}) filter {filterMode}");
                                            break;
                                    }
                                }
                                else
                                {
                                    InvalidCodePath($"Unknown filtering method {filterMethod}");
                                }

                                if (paletteIndex >= 0 && paletteIndex < palette?.Length)
                                {
                                    ref readonly ColorRGB8 paletteColor = ref palette[paletteIndex];
                                    r = paletteColor.r;
                                    g = paletteColor.g;
                                    b = paletteColor.b;

                                    if (paletteIndex < paletteAlpha?.Length)
                                    {
                                        a = paletteAlpha[paletteIndex];
                                    }
                                }
                                else
                                {
                                    InvalidCodePath($"pal idx out of range {paletteIndex} < {palette?.Length}");
                                }
                            }
                            break;

                        case PNGColorType.GreyscaleWithAlpha:
                            InvalidCodePath("Implement Greyscale with Alpha");
                            break;

                        case PNGColorType.TrueColorWithAlpha:
                            InvalidCodePath("Implement TrueColor with Alpha");
                            break;

                        default:
                            InvalidCodePath("Unknown PNG Color Type");
                            break;
                    }

                    return Color.FromArgb(a, r, g, b);
                }

                public void Dispose()
                {
                    compressedData?.Dispose();
                    rawData = null;
                    palette = null;
                    paletteAlpha = null;
                    text = null;
                }
            }

            private static readonly byte[] kPNGHeader = {
                0x89,
                0x50,
                0x4E,
                0x47,
                0x0D,
                0x0A,
                0x1A,
                0x0A,
            };

            private static uint FourCC(string str)
            {
                uint cc = 0;
                cc |= (uint)((byte)str[0] << 24);
                cc |= (uint)((byte)str[1] << 16);
                cc |= (uint)((byte)str[2] << 8);
                cc |= (uint)((byte)str[3] << 0);
                return cc;
            }

            private static string FourCCStr(uint cc)
            {
                Span<char> str = stackalloc char[5];
                str[0] = (char)(0xFF & (cc >> 24));
                str[1] = (char)(0xFF & (cc >> 16));
                str[2] = (char)(0xFF & (cc >> 8));
                str[3] = (char)(0xFF & (cc >> 0));
                str[4] = '\0';

                return new string(str);
            }

            private static int PaethPredictor(int a, int b, int c)
            {
                int p = a + b - c;
                int pa = Math.Abs(p - a);
                int pb = Math.Abs(p - b);
                int pc = Math.Abs(p - c);
                int pr = pa <= pb && pa <= pc ? a : pb <= pc ? b : c;
                return pr;
            }

            private static class PNGChunckType
            {
                // required
                public static readonly uint IHDR = FourCC(nameof(IHDR));
                public static readonly uint PLTE = FourCC(nameof(PLTE));
                public static readonly uint IDAT = FourCC(nameof(IDAT));
                public static readonly uint IEND = FourCC(nameof(IEND));
                // optional
                public static readonly uint tRNS = FourCC(nameof(tRNS));
                public static readonly uint cHRM = FourCC(nameof(cHRM));
                public static readonly uint gAMA = FourCC(nameof(gAMA));
                public static readonly uint iCCP = FourCC(nameof(iCCP));
                public static readonly uint sBIT = FourCC(nameof(sBIT));
                public static readonly uint sRGB = FourCC(nameof(sRGB));
                public static readonly uint cICP = FourCC(nameof(cICP));
                public static readonly uint mDCV = FourCC(nameof(mDCV));
                public static readonly uint cLLI = FourCC(nameof(cLLI));
                public static readonly uint tEXt = FourCC(nameof(tEXt));
                public static readonly uint iTXt = FourCC(nameof(iTXt));
                public static readonly uint zTXt = FourCC(nameof(zTXt));
                public static readonly uint bKGD = FourCC(nameof(bKGD));
                public static readonly uint hIST = FourCC(nameof(hIST));
                public static readonly uint pHYs = FourCC(nameof(pHYs));
                public static readonly uint sPLT = FourCC(nameof(sPLT));
                public static readonly uint eXIf = FourCC(nameof(eXIf));
                public static readonly uint tIME = FourCC(nameof(tIME));
                public static readonly uint acTL = FourCC(nameof(acTL));
                public static readonly uint fcTL = FourCC(nameof(fcTL));
                public static readonly uint fdAT = FourCC(nameof(fdAT));
            };

            delegate bool ParseChunkFunc(Stream stream, uint length, ref PNGContext context);

            private static readonly Dictionary<uint, ParseChunkFunc> kPNGParseChunkMap = new()
            {
                // required
                { PNGChunckType.IHDR, ParseChunk_IHDR },
                { PNGChunckType.PLTE, ParseChunk_PLTE },
                { PNGChunckType.IDAT, ParseChunk_IDAT },
                { PNGChunckType.IEND, ParseChunk_IEND },
                // optional
                { PNGChunckType.tRNS, ParseChunk_tRNS },
                { PNGChunckType.cHRM, ParseChunk_cHRM },
                { PNGChunckType.gAMA, ParseChunk_gAMA },
                { PNGChunckType.iCCP, ParseChunk_iCCP },
                { PNGChunckType.sBIT, ParseChunk_sBIT },
                { PNGChunckType.sRGB, ParseChunk_sRGB },
                { PNGChunckType.cICP, ParseChunk_cICP },
                { PNGChunckType.mDCV, ParseChunk_mDCV },
                { PNGChunckType.cLLI, ParseChunk_cLLI },
                { PNGChunckType.tEXt, ParseChunk_tEXt },
                { PNGChunckType.zTXt, ParseChunk_zTXt },
                { PNGChunckType.iTXt, ParseChunk_iTXt },
                { PNGChunckType.bKGD, ParseChunk_bKGD },
                { PNGChunckType.hIST, ParseChunk_hIST },
                { PNGChunckType.pHYs, ParseChunk_pHYs },
                { PNGChunckType.sPLT, ParseChunk_sPLT },
                { PNGChunckType.eXIf, ParseChunk_eXIf },
                { PNGChunckType.tIME, ParseChunk_tIME },
                { PNGChunckType.acTL, null },
                { PNGChunckType.fcTL, null },
                { PNGChunckType.fdAT, null },
            };

            private static bool TryReadUInt32(Stream stream, out uint value)
            {
                value = 0;
                Span<byte> data = stackalloc byte[4];
                if (stream.Read(data) == 4)
                {
                    value |= (uint)data[0] << 24;
                    value |= (uint)data[1] << 16;
                    value |= (uint)data[2] << 8;
                    value |= (uint)data[3] << 0;
                    return true;
                }

                return false;
            }

            private static bool TryReadUInt16(Stream stream, out ushort value)
            {
                value = 0;
                Span<byte> data = stackalloc byte[2];
                if (stream.Read(data) == 2)
                {
                    value |= (ushort)(data[0] << 8);
                    value |= (ushort)(data[1] << 0);
                    return true;
                }

                return false;
            }

            private static bool TryReadUInt8(Stream stream, out byte value)
            {
                value = 0;
                int data = stream.ReadByte();
                if (data >= 0)
                {
                    value = (byte)data;
                    return true;
                }

                return false;
            }

            private static bool TryReadString(Stream stream, out string str)
            {
                var auto = AutoStringBuilder.Auto();
                var sb = auto.sb;

                int data = stream.ReadByte();
                while (data > 0)
                {
                    sb.Append((char)data);
                    data = stream.ReadByte();
                }

                str = sb.ToString();
                return data == 0;
            }

            private static bool TryReadString(Stream stream, uint length, out string str)
            {
                var auto = AutoStringBuilder.Auto();
                var sb = auto.sb;

                str = string.Empty;

                for (uint i = 0; i < length; ++i)
                {
                    int data = stream.ReadByte();
                    if (data < 0)
                    {
                        return false;
                    }

                    sb.Append((char)data);
                }

                str = sb.ToString();
                return true;
            }

            private static bool TrySkip(Stream stream, uint length)
            {
                bool ok = true;

                for (uint i = 0; i < length; ++i)
                {
                    int read = stream.ReadByte();
                    if (read < 0)
                    {
                        ok = false;
                        break;
                    }
                }

                return ok;
            }

            private static bool ParseChunk_IHDR(Stream stream, uint length, ref PNGContext context)
            {
                bool ok = true;

                ok &= TryReadUInt32(stream, out uint width);
                ok &= TryReadUInt32(stream, out uint height);
                ok &= TryReadUInt8(stream, out byte bitDepth);
                ok &= TryReadUInt8(stream, out byte colorType);
                ok &= TryReadUInt8(stream, out byte compressionMethod);
                ok &= TryReadUInt8(stream, out byte filterMethod);
                ok &= TryReadUInt8(stream, out byte interlaceMethod);

                LogInfo("IHDR");
                if (ok)
                {
                    context.width = width;
                    context.height = height;
                    context.bitDepth = bitDepth;
                    context.colorType = (PNGColorType)colorType;
                    context.compressionMethod = (PNGCompressionMethod)compressionMethod;
                    context.filterMethod = (PNGFilterMethod)filterMethod;
                    context.interlaceMethod = (PNGInterlaceMethod)interlaceMethod;
                    LogInfo($"- {width} x {height} {bitDepth}bit {context.colorType} {context.compressionMethod} {context.filterMethod} {context.interlaceMethod}");
                }

                return ok;
            }

            private static bool ParseChunk_PLTE(Stream stream, uint length, ref PNGContext context)
            {
                Assert(length % 3 == 0);
                Assert(context.palette == null);

                uint entryCount = length / 3;
                context.palette = new ColorRGB8[entryCount];

                LogInfo("PLTE");
                LogInfo($"- {entryCount}");

                bool ok = true;
                for (uint i = 0; i < entryCount && ok; ++i)
                {
                    ColorRGB8 color;
                    ok &= TryReadUInt8(stream, out color.r);
                    ok &= TryReadUInt8(stream, out color.g);
                    ok &= TryReadUInt8(stream, out color.b);

                    if (ok)
                    {
                        context.palette[i] = color;
                    }
                }

                return ok;
            }

            private static bool ParseChunk_IDAT(Stream stream, uint length, ref PNGContext context)
            {
                bool ok = true;
                context.compressedData ??= new MemoryStream();

                LogInfo("IDAT");
                LogInfo($"- {length}");

                uint i = 0;
                while (i < length)
                {
                    int read = stream.ReadByte();
                    if (read < 0)
                    {
                        ok = false;
                        break;
                    }

                    context.compressedData.WriteByte((byte)read);
                    ++i;
                }

                return ok;
            }

            private static bool ParseChunk_IEND(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("IEND");
                return length == 0;
            }

            //

            private static bool ParseChunk_tRNS(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("tRNS");
                bool ok = true;
                switch (context.colorType)
                {
                    case PNGColorType.Greyscale:
                        ok &= TryReadUInt16(stream, out var greyValue);
                        break;

                    case PNGColorType.TrueColor:
                        ok &= TryReadUInt16(stream, out var redValue);
                        ok &= TryReadUInt16(stream, out var greenValue);
                        ok &= TryReadUInt16(stream, out var blueValue);
                        break;

                    case PNGColorType.IndexColor:
                        context.paletteAlpha = new byte[length];
                        for (uint i = 0; i < length && ok; ++i)
                        {
                            ok &= TryReadUInt8(stream, out var alphaValue);

                            if (ok)
                            {
                                context.paletteAlpha[i] = alphaValue;
                            }
                        }
                        break;

                    default:
                        InvalidCodePath();
                        break;
                }
                return ok;
            }

            private static bool ParseChunk_cHRM(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("cHRM");
                bool ok = true;

                ok &= TryReadUInt32(stream, out var whitePointX);
                ok &= TryReadUInt32(stream, out var whitePointY);

                ok &= TryReadUInt32(stream, out var redX);
                ok &= TryReadUInt32(stream, out var redY);

                ok &= TryReadUInt32(stream, out var greenX);
                ok &= TryReadUInt32(stream, out var greenY);

                ok &= TryReadUInt32(stream, out var blueX);
                ok &= TryReadUInt32(stream, out var blueY);

                if (ok)
                {
                    const float kScaleFactor = 100000.0f;
                }

                return ok;
            }

            private static bool ParseChunk_gAMA(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("gAMA");
                bool ok = true;

                ok &= TryReadUInt32(stream, out var imageGamma);

                if (ok)
                {
                    const float kScaleFactor = 100000.0f;
                    context.gamma = imageGamma / kScaleFactor;
                    LogInfo($"- {context.gamma}");
                }

                return ok;
            }

            private static bool ParseChunk_iCCP(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("iCCP");
                bool ok = true;

                ok &= TryReadUInt32(stream, out var imageGamma);

                if (ok)
                {
                    const float kScaleFactor = 100000.0f;
                    context.gamma = imageGamma / kScaleFactor;
                    LogInfo($"- {context.gamma}");
                }

                return ok;
            }

            private static bool ParseChunk_sBIT(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("sBIT");
                bool ok = true;

                switch (context.colorType)
                {
                    case PNGColorType.Greyscale:
                        ok &= TryReadUInt8(stream, out context.grayscaleSignificantBits);
                        break;
                    case PNGColorType.TrueColor:
                    case PNGColorType.IndexColor:
                        ok &= TryReadUInt8(stream, out context.redSignificantBits);
                        ok &= TryReadUInt8(stream, out context.greenSignificantBits);
                        ok &= TryReadUInt8(stream, out context.blueSignificantBits);
                        break;
                    case PNGColorType.GreyscaleWithAlpha:
                        ok &= TryReadUInt8(stream, out context.grayscaleSignificantBits);
                        ok &= TryReadUInt8(stream, out context.alphaSignificantBits);
                        break;
                    case PNGColorType.TrueColorWithAlpha:
                        ok &= TryReadUInt8(stream, out context.redSignificantBits);
                        ok &= TryReadUInt8(stream, out context.greenSignificantBits);
                        ok &= TryReadUInt8(stream, out context.blueSignificantBits);
                        ok &= TryReadUInt8(stream, out context.alphaSignificantBits);
                        break;
                    default:
                        InvalidCodePath();
                        break;
                }

                return ok;
            }

            private static bool ParseChunk_cICP(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("cICP");
                bool ok = true;

                ok &= TryReadUInt8(stream, out byte colorPrimaries);
                ok &= TryReadUInt8(stream, out byte transferFunction);
                ok &= TryReadUInt8(stream, out byte maxtrixCoeff);
                ok &= TryReadUInt8(stream, out byte videoFullRangeFlag);

                return ok;
            }

            private static bool ParseChunk_mDCV(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("mDCV");
                bool ok = true;

                ok &= TryReadUInt32(stream, out uint primaryChromaR); // x / 0.00002
                ok &= TryReadUInt32(stream, out uint primaryChromaG); // x / 0.00002
                ok &= TryReadUInt32(stream, out uint primaryChromaB); // x / 0.00002
                ok &= TryReadUInt32(stream, out uint whitePointChroma); // x / 0.00002
                ok &= TryReadUInt32(stream, out uint maxLuminance); // x / 0.0001
                ok &= TryReadUInt32(stream, out uint minLuminance); // x / 0.0001

                return ok;
            }

            private static bool ParseChunk_cLLI(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("cLLI");
                bool ok = true;

                ok &= TryReadUInt32(stream, out uint maxContentLightLevel); // x / 0.0001
                ok &= TryReadUInt32(stream, out uint maxFrameAverageLightLevel); // x / 0.0001

                return ok;
            }

            private static bool ParseChunk_sRGB(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("sRGB");
                bool ok = true;

                ok &= TryReadUInt8(stream, out var renderingIntent);

                if (ok)
                {
                    LogInfo($"- {renderingIntent}");
                }

                return ok;
            }

            private static bool ParseChunk_tIME(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("tIME");
                bool ok = true;

                ok &= TryReadUInt16(stream, out var year);
                ok &= TryReadUInt8(stream, out var month);
                ok &= TryReadUInt8(stream, out var day);
                ok &= TryReadUInt8(stream, out var hour);
                ok &= TryReadUInt8(stream, out var minute);
                ok &= TryReadUInt8(stream, out var second);

                return ok;
            }


            private static bool ParseChunk_tEXt(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("tEXt");
                bool ok = true;

                string keyword;
                string text = string.Empty;

                ok &= TryReadString(stream, out keyword);

                uint remaining = length - ((uint)keyword.Length + 1);
                if (remaining > 0)
                {
                    ok &= TryReadString(stream, remaining, out text);
                }

                if (ok)
                {
                    LogInfo($"- {keyword} -> '{text}'");
                    context.text ??= new();
                    context.text[keyword] = text;
                }

                return ok;
            }

            private static bool ParseChunk_zTXt(Stream stream, uint length, ref PNGContext context)
            {
                bool ok = true;

                string keyword;
                string text = string.Empty;

                ok &= TryReadString(stream, out keyword);

                uint remaining = length - (uint)keyword.Length;
                if (remaining > 0)
                {
                    ok &= TryReadUInt8(stream, out byte compressionMethod);

                    // TODO: decompress
                    ok &= TrySkip(stream, remaining);
                }

                if (ok)
                {
                    context.text ??= new();
                    context.text[keyword] = text;
                }

                return ok;
            }

            private static bool ParseChunk_iTXt(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("iTXt");
                bool ok = true;
                StringBuilder sb = new();

                string text = string.Empty;

                var p = stream.Position;

                ok &= TryReadString(stream, out string keyword);
                ok &= TryReadUInt8(stream, out byte compressionMethod);
                ok &= TryReadString(stream, out string languageTag);
                ok &= TryReadString(stream, out string translatedKeyword);

                uint remaining = (uint)(stream.Position - p) - length;
                if (remaining > 0)
                {
                    ok &= TryReadString(stream, remaining, out text);
                }

                if (ok)
                {
                    context.text ??= new();
                    context.text[keyword] = text;
                }

                return ok;
            }

            private static bool ParseChunk_bKGD(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("bKGB");
                bool ok = true;

                switch (context.colorType)
                {
                    case PNGColorType.Greyscale:
                    case PNGColorType.GreyscaleWithAlpha:
                        ok &= TryReadUInt16(stream, out var greyscaleBackground);
                        break;
                    case PNGColorType.TrueColor:
                    case PNGColorType.TrueColorWithAlpha:
                        ok &= TryReadUInt16(stream, out var redBackground);
                        ok &= TryReadUInt16(stream, out var greenBackground);
                        ok &= TryReadUInt16(stream, out var blueBackground);
                        break;
                    case PNGColorType.IndexColor:
                        ok &= TryReadUInt8(stream, out byte paletteIndexBackground);
                        break;
                    default:
                        InvalidCodePath();
                        break;
                }

                return ok;
            }
            private static bool ParseChunk_hIST(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("hIST");
                bool ok = true;

                for (uint i = 0; ok && i < length; ++i)
                {
                    ok &= TryReadUInt16(stream, out ushort frequency);
                }

                return ok;
            }
            private static bool ParseChunk_pHYs(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("pHYs");
                bool ok = true;

                ok &= TryReadUInt32(stream, out uint pixelsPerUnitX);
                ok &= TryReadUInt32(stream, out uint pixelsPerUnitY);
                ok &= TryReadUInt8(stream, out byte unitSpecifier); // 0 - unknown, 1 - meter

                if (ok)
                {
                    LogInfo($"- {pixelsPerUnitX} x {pixelsPerUnitY} @ {unitSpecifier}");
                }

                return ok;
            }

            private static bool ParseChunk_sPLT(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("sPLT");
                bool ok = true;

                for (int p = 0; ok && p < context.palette.Length; ++p)
                {
                    ok &= TryReadString(stream, out string paletteName);

                    ok &= TryReadUInt8(stream, out byte sampleDepth);
                    Assert(sampleDepth == 8 || sampleDepth == 16);
                    switch (sampleDepth)
                    {
                        case 8:
                            ok &= TryReadUInt8(stream, out var red);
                            ok &= TryReadUInt8(stream, out var green);
                            ok &= TryReadUInt8(stream, out var blue);
                            ok &= TryReadUInt8(stream, out var alpha);
                            break;
                        case 16:
                            ok &= TryReadUInt16(stream, out var _red);
                            ok &= TryReadUInt16(stream, out var _green);
                            ok &= TryReadUInt16(stream, out var _blue);
                            ok &= TryReadUInt16(stream, out var _alpha);
                            break;
                    }

                    ok &= TryReadUInt16(stream, out var frequency);
                }

                return ok;
            }

            private static bool ParseChunk_eXIf(Stream stream, uint length, ref PNGContext context)
            {
                LogInfo("eXIf");
                bool ok = true;

                ok &= TrySkip(stream, length);

                return ok;
            }

            private static void ParsePNG(Stream stream, List<Color> pixels, ref int width, ref int height)
            {
                PNGContext context = new();

                const int kHeaderSize = 8;
                Span<byte> pngHeader = stackalloc byte[kHeaderSize];
                int headerRead = stream.Read(pngHeader);
                Assert(headerRead == kHeaderSize);

                bool ok = true;
                bool isEndHeader = false;

                while (!isEndHeader && ok)
                {
                    ok &= TryReadUInt32(stream, out uint length);
                    ok &= TryReadUInt32(stream, out uint chunckType);

                    if (kPNGParseChunkMap.TryGetValue(chunckType, out var parseFunc))
                    {
                        ok &= parseFunc(stream, length, ref context);
                    }
                    else
                    {
                        LogWarning($"Unknown PNG Chunk Type {FourCCStr(chunckType)}");
                    }

                    ok &= TryReadUInt32(stream, out uint crc);

                    isEndHeader = chunckType == PNGChunckType.IEND;
                }

                if (ok)
                {
                    LogInfo("Ok");
                    width = (int)context.width;
                    height = (int)context.height;

                    context.Decompress();

                    pixels.Clear();
                    pixels.Capacity = width * height;

                    for (int y = 0; y < height; ++y)
                    {
                        for (int x = 0; x < width; ++x)
                        {
                            pixels.Add(context.GetColorAt(x, y));
                        }
                    }
                }

                context.Dispose();
            }
            #endregion

            #region TGA Parseing
            private static void ParseTGA(Stream stream, List<Color> pixels, ref int width, ref int height)
            {
                TryReadUInt8(stream, out var idLength);
                TryReadUInt8(stream, out var colorMapType);
                TryReadUInt8(stream, out var imageType);
                Span<byte> colorMapSepc = stackalloc byte[5];
                stream.Read(colorMapSepc);
                Span<byte> imageSpec = stackalloc byte[10];
                stream.Read(imageSpec);
            }
            #endregion

            #region BMP Parseing
            private static void ParseBMP(Stream stream, List<Color> pixels, ref int width, ref int height)
            {

            }
            #endregion
        }
        #endregion

        #region Util Structs
        [StructLayout(LayoutKind.Sequential)]
        unsafe struct SpritePalette : IEquatable<SpritePalette>
        {
            public string exportName;
            private fixed int c[4];

            public Color this[int index]
            {
                get => Color.FromArgb(c[index]);
                set => c[index] = value.ToArgb();
            }

            public void Order()
            {
                fixed (int* ptr = c)
                {
                    MemoryExtensions.Sort(new Span<int>(ptr, 4));
                }
            }

            public int IndexOf(Color color)
            {
                int argb = color.ToArgb();

                fixed (int* ptr = c)
                {
                    return MemoryExtensions.IndexOf(new Span<int>(ptr, 4), argb);
                }
            }

            public bool TryGetIndex(Color color, out int index)
            {
                index = IndexOf(color);
                return index >= 0;
            }

            public override int GetHashCode()
            {
                int hash = 17;
                hash = hash * 31 + c[0];
                hash = hash * 31 + c[1];
                hash = hash * 31 + c[2];
                hash = hash * 31 + c[3];
                return hash;
            }

            public override string ToString()
            {
                return $"({c[0]:X8}, {c[1]:X8}, {c[2]:X8}, {c[3]:X8})";
            }

            public bool Equals(SpritePalette other)
            {
                return c[0] == other.c[0] && c[1] == other.c[1] && c[2] == other.c[2] && c[3] == other.c[3];
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        unsafe struct TileIndices : IEquatable<TileIndices>
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

            public override bool Equals([NotNullWhen(true)] object obj)
            {
                return Equals((TileIndices)obj);
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

            public bool Equals(TileIndices other)
            {
                bool eq = true;
                for (int i = 0; i < 8 && eq; ++i)
                {
                    eq &= r[i] == other.r[i];
                }

                return eq;
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        struct TileEntry
        {
            public string tileName;
            public TileIndices tileIndices;
            public int spritePaletteIndex;
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
            public string language;
        }

        [StructLayout(LayoutKind.Sequential)]
        struct ChrRomOutput
        {
            public string chrRomAsm;
            public int chrCount;
            public bool is8x16;
        }

        enum TileAttribute
        {
            None = 0,
            FlipVertical = 2,
            FlipHorizontal = 1,
            FlipHorizontalVertical = 3,
        }

        [StructLayout(LayoutKind.Sequential)]
        struct UniqueTileEntry
        {
            public int index;
            public int attr;
        }
        #endregion

        #region Logging
        const char esc = (char)27;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        static void LogMessage(string msg)
        {
            Console.Out.WriteLine($"{esc}[92m{msg}{esc}[0m");
        }

        [Conditional("DEBUG")]
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        static void LogDebug(string msg)
        {
            Console.Out.WriteLine(msg);
        }

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

        #region Parameters
        static bool TryGetFileParamters(string srcFilename, string parameterFileExt, out Parameters parameters)
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

        static void WriteParametersToFile(Parameters parameters, string filename)
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

        static Parameters GetSubParameters(Parameters parameters, string key)
        {
            Parameters subparameters = new();

            if (!key.EndsWith('.'))
            {
                key += ".";
            }

            foreach (var kv in parameters)
            {
                if (kv.Key.StartsWith(key))
                {
                    string subkey = kv.Key.Substring(key.Length);
                    bool added = subparameters.TryAdd(subkey, kv.Value);
                    Assert(added, $"Unable to add sub parameter {kv.Key} -> {subkey}");
                }
            }

            return subparameters;
        }

        static bool GetBoolParameter(Parameters parameters, string key, bool defaultValue = false)
        {
            bool value = parameters?.TryGetValue(key, out string strValue) ?? false ? bool.TryParse(strValue, out bool boolValue) ? boolValue : defaultValue : defaultValue;
            return value;
        }

        static int GetIntParameter(Parameters parameters, string key, int defaultValue = 0)
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

        static long GetLongParameter(Parameters parameters, string key, long defaultValue = 0)
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

        static char GetCharParameter(Parameters parameters, string key, char defaultValue)
        {
            char value = parameters?.TryGetValue(key, out string strValue) ?? false ? !string.IsNullOrEmpty(strValue) ? strValue[0] : defaultValue : defaultValue;
            return value;
        }

        static string GetStringParameter(Parameters parameters, string key, string defaultValue = null)
        {
            string value = parameters?.TryGetValue(key, out string strValue) ?? false ? strValue : defaultValue;
            return value;
        }

        static void SetParameter<T>(Parameters parameters, string key, T value, Func<T, string> convertFunc = null)
        {
            static string DefaultFunc(T v) => v.ToString();
            if (parameters != null)
            {
                parameters[key] = (convertFunc ?? DefaultFunc)(value);
            }
        }
        #endregion

        #region Utils
        private static UTF8Encoding UTF8 = new UTF8Encoding(false);

        private static Stack<StringBuilder> s_StringBuilder = new();

        static StringBuilder RequestStringBuilder()
        {
            return s_StringBuilder.Count == 0 ? new StringBuilder() : s_StringBuilder.Pop();
        }

        static void ReleaseStringBuilder(StringBuilder sb)
        {
            sb.Clear();
            s_StringBuilder.Push(sb);
        }

        readonly struct AutoStringBuilder : IDisposable
        {
            public readonly StringBuilder sb;

            AutoStringBuilder(StringBuilder stringBuilder)
            {
                sb = stringBuilder;
            }

            public void Dispose()
            {
                ReleaseStringBuilder(sb);
            }

            public static AutoStringBuilder Auto()
            {
                return new AutoStringBuilder(RequestStringBuilder());
            }
        }

        static string PathCombine(params string[] paths)
        {
            using AutoStringBuilder auto = AutoStringBuilder.Auto();
            if (paths?.Length > 0)
            {
                if (string.IsNullOrEmpty(paths[0]))
                {
                    auto.sb.Append('.');
                }
                else
                {
                    auto.sb.Append(paths[0]);
                }

                for (int i = 1; i < paths.Length; i++)
                {
                    if (!string.IsNullOrEmpty(paths[i]))
                    {
                        auto.sb.Append(Path.DirectorySeparatorChar);
                        auto.sb.Append(paths[i]);
                    }
                }
            }

            return auto.sb.ToString();
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

        static bool TryParseRectString(string value, out Rect rect)
        {
            rect = default;

            bool parsed = false;
            if (!string.IsNullOrEmpty(value))
            {
                parsed = value.StartsWith('[') && value.EndsWith(']');
                if (parsed)
                {
                    var parts = value[1..^1].Split(',');
                    rect.origin.x = 0;
                    rect.origin.y = 0;
                    rect.size.width = 1;
                    rect.size.height = 1;

                    if (parts.Length >= 2)
                    {
                        int.TryParse(parts[0], out rect.origin.x);
                        int.TryParse(parts[1], out rect.origin.y);

                        if (parts.Length >= 4)
                        {
                            int.TryParse(parts[2], out rect.size.width);
                            int.TryParse(parts[3], out rect.size.height);
                        }
                    }
                }
            }

            return parsed;
        }

        static bool TryParseColorString(string value, out Color color)
        {
            color = default;

            bool parsed = false;
            if (!string.IsNullOrEmpty(value))
            {
                if (value.StartsWith('#'))
                {
                    parsed = int.TryParse(value[1..], out int argb);
                    color = Color.FromArgb(argb); ;
                }
                else if (value.StartsWith('$'))
                {
                    parsed = int.TryParse(value[1..], out int paletteId);
                    color = GetColorFromId(paletteId);
                }
                else if (value.StartsWith('[') && value.EndsWith(']'))
                {
                    var parts = value[1..^1].Split(',');
                    if (parts.Length >= 3)
                    {
                        bool ok = false;
                        ok &= int.TryParse(parts[0].Trim(), out int r);
                        ok &= int.TryParse(parts[1].Trim(), out int g);
                        ok &= int.TryParse(parts[2].Trim(), out int b);
                        int a = 255;
                        if (parts.Length >= 4)
                        {
                            ok &= int.TryParse(parts[3].Trim(), out a);
                        }

                        parsed = ok;
                        color = Color.FromArgb(a, r, g, b);
                    }
                }
            }

            return parsed;
        }
        #endregion

        #region Generate String Utils
        const string uint8Type = "uint8_t";
        const int kNameTableWidth = 32 * 8;
        const int kNameTableHeight = 30 * 8;

        // Palette from Lospec.com/palette-list
        // Palette Name: NES (Wikipedia)
        static Dictionary<Color, int> kNESColorToPalette = new()
        {
            { Color.FromArgb(unchecked((int)0xFF000000)), 0 },
            { Color.FromArgb(unchecked((int)0xFF666666)), 0 },
            { Color.FromArgb(unchecked((int)0xFFaeaeae)), 0 },
            { Color.FromArgb(unchecked((int)0xFF002a88)), 0 },
            { Color.FromArgb(unchecked((int)0xFF155fda)), 0 },
            { Color.FromArgb(unchecked((int)0xFF64b0fe)), 0 },
            { Color.FromArgb(unchecked((int)0xFFc1e0fe)), 0 },
            { Color.FromArgb(unchecked((int)0xFF1412a8)), 0 },
            { Color.FromArgb(unchecked((int)0xFF4240fe)), 0 },
            { Color.FromArgb(unchecked((int)0xFF9390fe)), 0 },
            { Color.FromArgb(unchecked((int)0xFFd4d3fe)), 0 },
            { Color.FromArgb(unchecked((int)0xFF3b00a4)), 0 },
            { Color.FromArgb(unchecked((int)0xFF7627ff)), 0 },
            { Color.FromArgb(unchecked((int)0xFFc777fe)), 0 },
            { Color.FromArgb(unchecked((int)0xFFe9c8fe)), 0 },
            { Color.FromArgb(unchecked((int)0xFF5c007e)), 0 },
            { Color.FromArgb(unchecked((int)0xFFa11bcd)), 0 },
            { Color.FromArgb(unchecked((int)0xFFf36afe)), 0 },
            { Color.FromArgb(unchecked((int)0xFFfbc3fe)), 0 },
            { Color.FromArgb(unchecked((int)0xFF6e0040)), 0 },
            { Color.FromArgb(unchecked((int)0xFFb81e7c)), 0 },
            { Color.FromArgb(unchecked((int)0xFFfe6ecd)), 0 },
            { Color.FromArgb(unchecked((int)0xFFfec5eb)), 0 },
            { Color.FromArgb(unchecked((int)0xFF6c0700)), 0 },
            { Color.FromArgb(unchecked((int)0xFFb53220)), 0 },
            { Color.FromArgb(unchecked((int)0xFFfe8270)), 0 },
            { Color.FromArgb(unchecked((int)0xFFfecdc6)), 0 },
            { Color.FromArgb(unchecked((int)0xFF571d00)), 0 },
            { Color.FromArgb(unchecked((int)0xFF994f00)), 0 },
            { Color.FromArgb(unchecked((int)0xFFeb9f23)), 0 },
            { Color.FromArgb(unchecked((int)0xFFf7d9a6)), 0 },
            { Color.FromArgb(unchecked((int)0xFF343500)), 0 },
            { Color.FromArgb(unchecked((int)0xFF6c6e00)), 0 },
            { Color.FromArgb(unchecked((int)0xFFbdbf00)), 0 },
            { Color.FromArgb(unchecked((int)0xFFe5e695)), 0 },
            { Color.FromArgb(unchecked((int)0xFF0c4900)), 0 },
            { Color.FromArgb(unchecked((int)0xFF388700)), 0 },
            { Color.FromArgb(unchecked((int)0xFF89d900)), 0 },
            { Color.FromArgb(unchecked((int)0xFFd0f097)), 0 },
            { Color.FromArgb(unchecked((int)0xFF005200)), 0 },
            { Color.FromArgb(unchecked((int)0xFF0d9400)), 0 },
            { Color.FromArgb(unchecked((int)0xFF5de530)), 0 },
            { Color.FromArgb(unchecked((int)0xFFbef5ab)), 0 },
            { Color.FromArgb(unchecked((int)0xFF004f08)), 0 },
            { Color.FromArgb(unchecked((int)0xFF009032)), 0 },
            { Color.FromArgb(unchecked((int)0xFF45e182)), 0 },
            { Color.FromArgb(unchecked((int)0xFFb4f3cd)), 0 },
            { Color.FromArgb(unchecked((int)0xFF00404e)), 0 },
            { Color.FromArgb(unchecked((int)0xFF007c8e)), 0 },
            { Color.FromArgb(unchecked((int)0xFF48cedf)), 0 },
            { Color.FromArgb(unchecked((int)0xFFb5ecf3)), 0 },
            { Color.FromArgb(unchecked((int)0xFF4f4f4f)), 0 },
            { Color.FromArgb(unchecked((int)0xFFb8b8b8)), 0 },
            { Color.FromArgb(unchecked((int)0xFFfefefe)), 0 },
        };

        // Colors from Aseprite NES palette
        static Dictionary<Color, int> kAsepriteNESPalette = new() {
            { Color.FromArgb(0, 0, 0), 0 },
            { Color.FromArgb(121, 121, 121),0 },
            { Color.FromArgb(162, 162, 162),0 },
            { Color.FromArgb(48, 81, 130),0 },
            { Color.FromArgb(65, 146, 195),0 },
            { Color.FromArgb(97, 211, 227),0 },
            { Color.FromArgb(162, 255, 243),0 },
            { Color.FromArgb(48, 97, 65),0 },
            { Color.FromArgb(73, 162, 105),0 },
            { Color.FromArgb(113, 227, 146),0 },
            { Color.FromArgb(162, 255, 203),0 },
            { Color.FromArgb(56, 109, 0),0 },
            { Color.FromArgb(73, 170, 16),0 },
            { Color.FromArgb(113, 243, 65),0 },
            { Color.FromArgb(162, 243, 162),0 },
            { Color.FromArgb(56, 105, 0),0 },
            { Color.FromArgb(81, 162, 0),0 },
            { Color.FromArgb(154, 235, 0),0 },
            { Color.FromArgb(203, 243, 130),0 },
            { Color.FromArgb(73, 89, 0),0 },
            { Color.FromArgb(138, 138, 0),0 },
            { Color.FromArgb(235, 211, 32),0 },
            { Color.FromArgb(255, 243, 146),0 },
            { Color.FromArgb(121, 65, 0),0 },
            { Color.FromArgb(195, 113, 0),0 },
            { Color.FromArgb(255, 162, 0),0 },
            { Color.FromArgb(255, 219, 162),0 },
            { Color.FromArgb(162, 48, 0),0 },
            { Color.FromArgb(227, 81, 0),0 },
            { Color.FromArgb(255, 121, 48),0 },
            { Color.FromArgb(255, 203, 186),0 },
            { Color.FromArgb(178, 16, 48),0 },
            { Color.FromArgb(219, 65, 97),0 },
            { Color.FromArgb(255, 97, 178),0 },
            { Color.FromArgb(255, 186, 235),0 },
            { Color.FromArgb(154, 32, 121),0 },
            { Color.FromArgb(219, 65, 195),0 },
            { Color.FromArgb(243, 97, 255),0 },
            { Color.FromArgb(227, 178, 255),0 },
            { Color.FromArgb(97, 16, 162),0 },
            { Color.FromArgb(146, 65, 243),0 },
            { Color.FromArgb(162, 113, 255),0 },
            { Color.FromArgb(195, 178, 255),0 },
            { Color.FromArgb(40, 0, 186),0 },
            { Color.FromArgb(65, 65, 255),0 },
            { Color.FromArgb(81, 130, 255),0 },
            { Color.FromArgb(162, 186, 255),0 },
            { Color.FromArgb(32, 0, 178),0 },
            { Color.FromArgb(65, 97, 251),0 },
            { Color.FromArgb(97, 162, 255),0 },
            { Color.FromArgb(146, 211, 255),0 },
            { Color.FromArgb(178, 178, 178),0 },
            { Color.FromArgb(235, 235, 235),0 },
            { Color.FromArgb(255, 255, 255),0 },
        };

        private static int KnownColorToId(Color color)
        {
            return IsKnownColor(color, out int id) ? id : 0;
        }

        private static bool IsKnownColor(Color color, out int id)
        {
            bool known = kAsepriteNESPalette.TryGetValue(color, out id);
            if (!known)
            {
                known = kNESColorToPalette.TryGetValue(color, out id);
            }

            return known;
        }

        private static Color GetColorFromId(int id)
        {
            return Color.AliceBlue;
        }

        private static (List<TileEntry> tiles, List<SpritePalette> spritePalettes) ConvertImageToTiles(ImageBitmap bitmapImage, Rectangle area, int paletteWidth, int paletteHeight, Color? backgroundColor = null)
        {
            List<TileEntry> allTiles = new List<TileEntry>();

            Span<int> palettesUsed = stackalloc int[4];

            HashSet<SpritePalette> uniqueSpritePalettes = new();
            List<(Rectangle area, SpritePalette palette)> allSpritePalettes = new();

            HashSet<Color> uniqueAttributeColors = new();

            // generate palattes from image
            for (int y = area.Y, ymax = area.Y + area.Height; y < ymax; y += paletteHeight)
            {
                for (int x = area.X, xmax = area.X + area.Width; x < xmax; x += paletteWidth)
                {
                    uniqueAttributeColors.Clear();

                    for (int py = 0; py < paletteHeight; ++py)
                    {
                        for (int px = 0; px < paletteWidth; ++px)
                        {
                            int vx = x + px;
                            int vy = y + py;
                            if (vx < bitmapImage.Width && vy < bitmapImage.Height)
                            {
                                Color pixel = bitmapImage.GetPixel(vx, vy);
                                Assert(IsKnownColor(pixel, out var _), $"Unknown Color at [{vx}, {vy}]: {pixel} ({pixel.ToArgb():X8})");

                                uniqueAttributeColors.Add(pixel);
                            }
                        }
                    }

                    Assert(uniqueAttributeColors.Count <= 4, $"Too may colors ({uniqueAttributeColors.Count}) in 16x16 region ({x}, {y}) to form palette");

                    SpritePalette spritePalette = default;

                    int idx = 0;
                    foreach (Color c in uniqueAttributeColors)
                    {
                        spritePalette[idx] = c;
                        ++idx;

                        if (idx >= 4) break;
                    }

                    for (; idx < 4; ++idx)
                    {
                        spritePalette[idx] = Color.Black;
                    }

                    // order palette so it can be uniquely defined
                    spritePalette.Order();

                    // add sprite palette region
                    uniqueSpritePalettes.Add(spritePalette);
                    allSpritePalettes.Add((new Rectangle(x, y, paletteWidth, paletteHeight), spritePalette));
                }
            }

            //Assert(uniqueSpritePalettes.Count <= 4, $"Too many defined sprite palettes ({uniqueSpritePalettes.Count})");

            //foreach (SpritePalette spritePalette in uniqueSpritePalettes)
            //{
            //LogInfo(spritePalette.ToString());
            //}

            List<SpritePalette> allTileSpritePalettes = new(uniqueSpritePalettes);

            // read tiles
            for (int y = area.Y, ymax = area.Y + area.Height; y < ymax; y += 8)
            {
                for (int x = area.X, xmax = area.X + area.Width; x < xmax; x += 8)
                {
                    TileIndices tileIndices = default;

                    (Rectangle _, SpritePalette spritePalette) = allSpritePalettes.Find(p => p.area.Contains(x, y));

                    for (int py = 0; py < 8; ++py)
                    {
                        for (int px = 0; px < 8; ++px)
                        {
                            int vx = x + px;
                            int vy = y + py;
                            if (vx < bitmapImage.Width && vy < bitmapImage.Height)
                            {
                                Color pixel = bitmapImage.GetPixel(vx, vy);

                                bool found = spritePalette.TryGetIndex(pixel, out int indexInPalette);
                                Assert(found, $"Unable to find index for color {pixel} at [{vx}, {vy}]");

                                if (found)
                                {
                                    tileIndices[px, py] = indexInPalette;
                                }
                            }
                        }
                    }

                    int spritePaletteIndex = allTileSpritePalettes.IndexOf(spritePalette);

                    allTiles.Add(new()
                    {
                        x = x,
                        y = y,
                        tileIndices = tileIndices,
                        spritePaletteIndex = spritePaletteIndex,
                    });
                }
            }

            // if there's a bg color, remove solid bg colors
            if (backgroundColor.HasValue)
            {
                SpritePalette fullBackgroundPalette = default;
                fullBackgroundPalette[0] = backgroundColor.Value;
                fullBackgroundPalette[1] = backgroundColor.Value;
                fullBackgroundPalette[2] = backgroundColor.Value;
                fullBackgroundPalette[3] = backgroundColor.Value;

                int backgroundPaletteIndex = allTileSpritePalettes.IndexOf(fullBackgroundPalette);
                if (backgroundPaletteIndex >= 0)
                {
                    // remove the bg-only palette
                    allTileSpritePalettes.RemoveAt(backgroundPaletteIndex);

                    int prevTileCount = allTiles.Count;

                    // remove all bg-only tiles
                    allTiles.RemoveAll(x => x.spritePaletteIndex == backgroundPaletteIndex);

                    LogInfo($"Removed background-only ({backgroundColor.Value}) tiles {prevTileCount} -> {allTiles.Count}");

                    // adjust tiles from the removed bg
                    for (int i = 0; i < allTiles.Count; i++)
                    {
                        TileEntry tile = allTiles[i];
                        if (tile.spritePaletteIndex > backgroundPaletteIndex)
                        {
                            tile.spritePaletteIndex--;
                            allTiles[i] = tile;
                        }
                    }
                }
            }

            return (allTiles, allTileSpritePalettes);
        }

        private static string NormalizeExportName(string name)
        {
            return name.ToLowerInvariant().Replace(' ', '_').Replace('.', '_').Replace('-', '_');
        }

        const string kRODATASegment = "RODATA";

        private static void WriteGeneratedAsmFile(StringBuilder sb, string filename, in ConvertOptions cmdOptions)
        {
            if (sb.Length > 0)
            {
                try
                {
                    string sFilename = PathCombine(Path.GetDirectoryName(filename), cmdOptions.defaultGeneratedAssetFolder, "asm", $"{Path.GetFileName(filename)}.s");
                    File.WriteAllText(sFilename, sb.ToString(), UTF8);

                    LogInfo($"Converted {filename} -> {sFilename}");
                }
                catch (Exception e)
                {
                    LogError(e.Message);
                }
            }
        }

        private static void WriteGeneratedHeaderFile(StringBuilder sb, string filename, in ConvertOptions cmdOptions)
        {
            if (sb.Length > 0)
            {
                try
                {
                    string hFilename = PathCombine(Path.GetDirectoryName(filename), cmdOptions.defaultGeneratedAssetFolder, "include", $"{Path.GetFileName(filename)}.h");
                    File.WriteAllText(hFilename, sb.ToString(), UTF8);

                    LogInfo($"Converted {filename} -> {hFilename}");
                }
                catch (Exception e)
                {
                    LogError(e.Message);
                }
            }
        }

        private static void GenerateAsmFileHeader(StringBuilder sb, string srcFilename, string segment = null)
        {
            sb.AppendLine(";");
            sb.AppendLine($"; Generated from {Path.GetFileName(srcFilename)}");
            sb.AppendLine(";");
            sb.AppendLine();

            if (!string.IsNullOrEmpty(segment))
            {
                sb.AppendLine($".segment \"{segment}\"");
                sb.AppendLine();
            }
        }

        private static void WrapHeaderFile(StringBuilder sb, string srcFilename, string dstFilename)
        {
            using var auto = AutoStringBuilder.Auto();

            auto.sb.AppendLine("//");
            auto.sb.AppendLine($"// Generated from {Path.GetFileName(srcFilename)}");
            auto.sb.AppendLine("//");
            auto.sb.AppendLine();

            string defineGuard = $"{Path.GetFileName(dstFilename)}.h".Replace('-', '_').Replace('.', '_').ToUpperInvariant();
            auto.sb.AppendLine($"#ifndef {defineGuard}");
            auto.sb.AppendLine($"#define {defineGuard}");
            auto.sb.AppendLine();

            sb.Insert(0, auto.sb.ToString());

            sb.AppendLine();
            sb.AppendLine($"#endif // {defineGuard}");
        }

        private static void GeneratePaletteString(StringBuilder sb, string exportName, IEnumerable<SpritePalette> spritePalettes)
        {
            sb.AppendLine($".export {exportName}_palettes = _{exportName}_palettes");
            sb.AppendLine($"_{exportName}_palettes:");
            sb.AppendLine();

            int idx = 0;
            foreach (SpritePalette spritePalette in spritePalettes)
            {
                string export = string.IsNullOrEmpty(spritePalette.exportName) ? $"{exportName}_palette_{idx}" : spritePalette.exportName;

                sb.AppendLine($"; Palette {idx}:");
                sb.AppendLine($";  0: ${KnownColorToId(spritePalette[0]):X2} ({spritePalette[0]})");
                sb.AppendLine($";  1: ${KnownColorToId(spritePalette[1]):X2} ({spritePalette[1]})");
                sb.AppendLine($";  2: ${KnownColorToId(spritePalette[2]):X2} ({spritePalette[2]})");
                sb.AppendLine($";  3: ${KnownColorToId(spritePalette[3]):X2} ({spritePalette[3]})");
                sb.AppendLine();

                sb.AppendLine($".export {export} = _{export}");
                sb.AppendLine($"_{export}:");

                sb.Append(".byte");
                sb.Append($" #${KnownColorToId(spritePalette[0]):X2},");
                sb.Append($" #${KnownColorToId(spritePalette[1]):X2},");
                sb.Append($" #${KnownColorToId(spritePalette[2]):X2},");
                sb.Append($" #${KnownColorToId(spritePalette[3]):X2}");
                sb.AppendLine();

                ++idx;
            }
        }

        private static int PixelsToTiles(int pixels) => pixels / 8;

        private static int PackPaletteOffsetLength(bool compressed, int offset, int count)
        {
            return (compressed ? 0x80 : 0) | (0x3 & count) | ((0x3 & offset) << 2);
        }

        private static void GenerateBackgroundHeaderString(StringBuilder sb, string exportName, int nametable, int offsetX, int offsetY, int width, int height, bool compressed, int paletteOffset, int paletteCount)
        {
            sb.AppendLine($"; Background {exportName}");
            sb.AppendLine($".export {exportName}_bg = _{exportName}_bg");
            sb.AppendLine($"_{exportName}_bg:");
            sb.AppendLine($".byte #${nametable:X2} ; {nameof(nametable)}");
            sb.AppendLine($".byte #${PixelsToTiles(offsetX):X2} ; {nameof(offsetX)} in tiles");
            sb.AppendLine($".byte #${PixelsToTiles(offsetY):X2} ; {nameof(offsetY)} in tiles");
            sb.AppendLine($".byte #${PixelsToTiles(width):X2} ; {nameof(width)} in tiles");
            sb.AppendLine($".byte #${PixelsToTiles(height):X2} ; {nameof(height)} in tiles");
            sb.AppendLine($".byte #${PackPaletteOffsetLength(compressed, paletteOffset, paletteCount):X2} ; %z000xxyy z={nameof(compressed)} x={nameof(paletteOffset)} y={nameof(paletteCount)}");
            sb.AppendLine($".addr {exportName}_palettes ; base palette address");
            sb.AppendLine($".addr {exportName}_tiles ; base tile address");
            sb.AppendLine();
        }

        class TileEntryComparer : IEqualityComparer<TileEntry>
        {
            public bool Equals(TileEntry x, TileEntry y)
            {
                return x.tileIndices.Equals(y.tileIndices);
            }

            public int GetHashCode([DisallowNull] TileEntry obj)
            {
                return obj.GetHashCode();
            }
        }

        private static void GenerateBackgroundString(StringBuilder sb, string exportName, Dictionary<string, ChrRomOutput> outputChrData, IEnumerable<TileEntry> tileEntries)
        {
            List<TileEntry> uniqueTileEntries = new();

            List<int> bytes = new();

            foreach (var tileEntry in tileEntries)
            {
                int tileIndex = uniqueTileEntries.FindIndex(x => x.tileIndices.Equals(tileEntry.tileIndices));
                if (tileIndex < 0)
                {
                    tileIndex = uniqueTileEntries.Count;
                    uniqueTileEntries.Add(tileEntry);
                }

                bytes.Add(tileIndex);
            }

            using (var auto = AutoStringBuilder.Auto())
            {
                int rleBytes = 0;

                auto.sb.AppendLine("; tile index");

                foreach (int idx in bytes)
                {
                    auto.sb.AppendLine($".byte #${idx:X2}");
                    rleBytes++;

                    if (rleBytes == 255)
                    {
                        sb.AppendLine($".byte #${rleBytes:X2} ; Bytes");
                        sb.AppendLine(auto.sb.ToString());

                        rleBytes = 0;
                        auto.sb.Clear();

                        auto.sb.AppendLine("; tile count, tile index");
                    }
                }

                if (rleBytes > 0)
                {
                    sb.AppendLine($".byte #${rleBytes:X2} ; Bytes");
                    sb.AppendLine(auto.sb.ToString());
                }
            }

            // output CHR for tiles
            using (AutoStringBuilder auto = AutoStringBuilder.Auto())
            {
                GenerateTileCHRString(auto.sb, exportName, uniqueTileEntries);

                // output chr data
                outputChrData.Add(exportName, new()
                {
                    chrRomAsm = auto.sb.ToString(),
                    chrCount = uniqueTileEntries.Count,
                    is8x16 = false
                });
            }
        }

        private static void GenerateCompressedBackgroundString(StringBuilder sb, string exportName, Dictionary<string, ChrRomOutput> outputChrData, IEnumerable<TileEntry> tileEntries)
        {
            List<TileEntry> uniqueTileEntries = new();

            int currentTileIndex = -1;
            int tileIndexCount = 0;

            List<(int, int)> compressedBytes = new();

            foreach (var tileEntry in tileEntries)
            {
                int tileIndex = uniqueTileEntries.FindIndex(x => x.tileIndices.Equals(tileEntry.tileIndices));
                if (tileIndex < 0)
                {
                    tileIndex = uniqueTileEntries.Count;
                    uniqueTileEntries.Add(tileEntry);
                }

                if (currentTileIndex != tileIndex || tileIndexCount == 0xFF)
                {
                    if (tileIndexCount > 0 && currentTileIndex >= 0)
                    {
                        compressedBytes.Add((tileIndexCount, currentTileIndex));

                    }

                    currentTileIndex = tileIndex;
                    tileIndexCount = 1;
                }
                else
                {
                    ++tileIndexCount;
                }
            }


            using (var auto = AutoStringBuilder.Auto())
            {
                int rleBytes = 0;

                auto.sb.AppendLine("; tile count, tile index");

                foreach ((int count, int idx) in compressedBytes)
                {
                    auto.sb.AppendLine($".byte #${count:X2}, #${idx:X2}");
                    rleBytes += 2;

                    if (rleBytes == 254)
                    {
                        sb.AppendLine($".byte #${rleBytes:X2} ; RLE Bytes");
                        sb.AppendLine(auto.sb.ToString());

                        rleBytes = 0;
                        auto.sb.Clear();

                        auto.sb.AppendLine("; tile count, tile index");
                    }
                }

                if (rleBytes > 0)
                {
                    sb.AppendLine($".byte #${rleBytes:X2} ; RLE Bytes");
                    sb.AppendLine(auto.sb.ToString());
                }
            }

            // output CHR for tiles
            using (AutoStringBuilder auto = AutoStringBuilder.Auto())
            {
                GenerateTileCHRString(auto.sb, exportName, uniqueTileEntries);

                // output chr data
                outputChrData.Add(exportName, new()
                {
                    chrRomAsm = auto.sb.ToString(),
                    chrCount = uniqueTileEntries.Count,
                    is8x16 = false
                });
            }
        }

        private static void GenerateTileCHRString(StringBuilder sb, string exportName, IEnumerable<TileEntry> tileEntries)
        {
            sb.AppendLine($".export {exportName}_tiles = _{exportName}_tiles");
            sb.AppendLine($"_{exportName}_tiles:");
            sb.AppendLine();

            StringBuilder byte0 = RequestStringBuilder();
            StringBuilder byte1 = RequestStringBuilder();

            int idx = 0;
            foreach (TileEntry tileEntry in tileEntries)
            {
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

                string tileEntryName = string.IsNullOrEmpty(tileEntry.tileName) ? $"{exportName}_tile_{tileEntry.x}x{tileEntry.y}" : tileEntry.tileName;
                sb.AppendLine($"; Tile {idx} {tileEntry.x}x{tileEntry.y} ({tileEntry.tileName ?? string.Empty})");
                sb.AppendLine($".export {tileEntryName} = _{tileEntryName}");
                sb.AppendLine($"_{tileEntryName}:");
                sb.AppendLine(byte0.ToString());
                sb.AppendLine(byte1.ToString());

                ++idx;
            }

            sb.AppendLine();

            ReleaseStringBuilder(byte0);
            ReleaseStringBuilder(byte1);
        }
        #endregion

        #region Palette File
        private static void ConvertPaletteFile(string paletteFilename, Dictionary<string, ChrRomOutput> outputChrData, in ConvertOptions cmdOptions)
        {
            TryGetFileParamters(paletteFilename, string.Empty, out var paletteParameters);

            if (!GetBoolParameter(paletteParameters, "enable-import", true))
            {
                return;
            }

            Parameters palettes = GetSubParameters(paletteParameters, "palette");

            Dictionary<string, SpritePalette> spritePalettes = new();
            foreach (var kv in palettes)
            {
                string paletteName = kv.Key[0..^2];
                Parameters paletteParts = GetSubParameters(palettes, paletteName);

                TryParseColorString(GetStringParameter(paletteParts, "0"), out var color0);
                TryParseColorString(GetStringParameter(paletteParts, "1"), out var color1);
                TryParseColorString(GetStringParameter(paletteParts, "2"), out var color2);
                TryParseColorString(GetStringParameter(paletteParts, "3"), out var color3);

                SpritePalette spritePalette = default;
                spritePalette.exportName = paletteName;
                spritePalette[0] = color0;
                spritePalette[1] = color1;
                spritePalette[2] = color2;
                spritePalette[3] = color3;

                spritePalettes.Add(paletteName, spritePalette);
            }

            if (spritePalettes.Count > 0)
            {
                string exportName = NormalizeExportName(Path.GetFileName(paletteFilename));

                // write background info to file
                using (AutoStringBuilder auto = AutoStringBuilder.Auto())
                {
                    GenerateAsmFileHeader(auto.sb, paletteFilename, kRODATASegment);

                    GeneratePaletteString(auto.sb, exportName, spritePalettes.Values);

                    WriteGeneratedAsmFile(auto.sb, paletteFilename, in cmdOptions);
                }
            }
        }
        #endregion

        #region Background File
        private static void ConvertBackgroundImageFile(string backgroundFilename, Dictionary<string, ChrRomOutput> outputChrData, in ConvertOptions cmdOptions)
        {
            TryGetFileParamters(backgroundFilename, string.Empty, out var backgroundParameters);

            if (!GetBoolParameter(backgroundParameters, "enable-import", true))
            {
                return;
            }

            string dir = Path.GetDirectoryName(backgroundFilename);
            string imageFilename = Path.GetFileNameWithoutExtension(backgroundFilename);

            FileStreamOptions fileOptions = new()
            {
                Mode = FileMode.Open,
                Access = FileAccess.Read
            };

            using var bitmapImage = new ImageBitmap(new FileStream(PathCombine(dir, imageFilename), fileOptions));

            int backgroundWidth = GetIntParameter(backgroundParameters, "bg.width", kNameTableWidth);
            int backgroundHeight = GetIntParameter(backgroundParameters, "bg.height", kNameTableHeight);
            int backgroundX = GetIntParameter(backgroundParameters, "bg.x", 0);
            int backgroundY = GetIntParameter(backgroundParameters, "bg.y", 0);
            int baseNameTable = GetIntParameter(backgroundParameters, "bg.nametable", 0);
            string backgroundColorString = GetStringParameter(backgroundParameters, "bg.color", "[0,0,0]");
            bool compress = GetBoolParameter(backgroundParameters, "bg.compress", true);

            Assert(bitmapImage.Width % backgroundWidth == 0, $"Image width must be mulitple of {backgroundWidth}");
            Assert(bitmapImage.Height % backgroundHeight == 0, $"Image height must be mulitple of {backgroundHeight}");
            Assert(bitmapImage.Width <= kNameTableWidth * 2, $"Image width too much for 4 nametables: {bitmapImage.Width}");
            Assert(bitmapImage.Height <= kNameTableHeight * 2, $"Image height too much for 4 nametables: {bitmapImage.Height}");

            TryParseColorString(backgroundColorString, out var bgColor);

            var area = new Rectangle(0, 0, bitmapImage.Width, bitmapImage.Height);
            var tiles = ConvertImageToTiles(bitmapImage, area, 16, 16, bgColor);

            var offset = new Rectangle(tiles.tiles[0].x, tiles.tiles[0].y, 8, 8);
            for (int i = 1; i < tiles.tiles.Count; i++)
            {
                TileEntry t = tiles.tiles[i];
                offset = Rectangle.Union(offset, new Rectangle(t.x, t.y, 8, 8));
            }

            string exportName = NormalizeExportName(Path.GetFileName(backgroundFilename));

            // write background info to file
            using (AutoStringBuilder auto = AutoStringBuilder.Auto())
            {
                GenerateAsmFileHeader(auto.sb, backgroundFilename, kRODATASegment);

                GenerateBackgroundHeaderString(auto.sb, exportName, baseNameTable, offset.X, offset.Y, offset.Width, offset.Height, compress, 0, tiles.spritePalettes.Count);

                // output CHR for tiles
                if (compress)
                {
                    GenerateCompressedBackgroundString(auto.sb, exportName, outputChrData, tiles.tiles);
                }
                else
                {
                    GenerateBackgroundString(auto.sb, exportName, outputChrData, tiles.tiles);
                }

                GeneratePaletteString(auto.sb, exportName, tiles.spritePalettes);

                WriteGeneratedAsmFile(auto.sb, backgroundFilename, in cmdOptions);
            }
        }
        #endregion

        #region Sprite File
        static void ConvertSpriteFile(string spriteFilename, Dictionary<string, ChrRomOutput> outputChrData, in ConvertOptions cmdOptions)
        {
            TryGetFileParamters(spriteFilename, string.Empty, out var spriteParameters);

            if (!GetBoolParameter(spriteParameters, "enable-import", true))
            {
                return;
            }

            string dir = Path.GetDirectoryName(spriteFilename);
            string imageFilename = Path.GetFileNameWithoutExtension(spriteFilename);

            FileStreamOptions fileOptions = new()
            {
                Mode = FileMode.Open,
                Access = FileAccess.Read
            };

            using var bitmapImage = new ImageBitmap(new FileStream(PathCombine(dir, imageFilename), fileOptions));

            Assert(bitmapImage.Width >= 8, $"Image too small, width: {bitmapImage.Width} requried at least 8px");
            Assert(bitmapImage.Height >= 8, $"Image too small, height: {bitmapImage.Height} requried at least 8px");

            int metaSpriteWidth = GetIntParameter(spriteParameters, "meta-sprite.width");
            int metaSpriteHeight = GetIntParameter(spriteParameters, "meta-sprite.height");

            Rectangle area = new Rectangle(0, 0, bitmapImage.Width, bitmapImage.Height);

            area.X = Math.Clamp(GetIntParameter(spriteParameters, "image.max-rect.x", area.X), 0, bitmapImage.Width);
            area.Y = Math.Clamp(GetIntParameter(spriteParameters, "image.max-rect.y", area.Y), 0, bitmapImage.Height);
            area.Width = Math.Clamp(GetIntParameter(spriteParameters, "image.max-rect.w", area.Width), 0, bitmapImage.Width);
            area.Height = Math.Clamp(GetIntParameter(spriteParameters, "image.max-rect.h", area.Height), 0, bitmapImage.Height);

            var tiles = ConvertImageToTiles(bitmapImage, area, 8, 8);

            string exportName = NormalizeExportName(Path.GetFileName(spriteFilename));

            // remove duplcate tiles
            List<int> uniqueTileIndices = new(tiles.tiles.Count);
            List<UniqueTileEntry> allUniqueTiles = new(tiles.tiles.Count);
            Dictionary<int, UniqueTileEntry> uniqueTiles = new(tiles.tiles.Count);

            bool removeDuplicateSprites = GetBoolParameter(spriteParameters, "sprite.remove-duplicates", true);
            if (removeDuplicateSprites)
            {
                Dictionary<int, int> allTilesToUniqueTiles = new(tiles.tiles.Count);
                Dictionary<int, int> uniqueTileHashToIndex = new(tiles.tiles.Count);
                HashSet<int> uniqueTileHashes = new(tiles.tiles.Count);

                // bucket each tile hash
                for (int i = 0; i < tiles.tiles.Count; i++)
                {
                    TileEntry tileEntry = tiles.tiles[i];

                    int tileHash = tileEntry.tileIndices.GetHashCode();
                    int tileHashV = tileEntry.tileIndices.FlipVertical().GetHashCode();
                    int tileHashH = tileEntry.tileIndices.FlipHorizontal().GetHashCode();
                    int tileHashVH = tileEntry.tileIndices.FlipHorizontalVertical().GetHashCode();

                    bool added = false;
                    added |= uniqueTiles.TryAdd(tileHash, new() { index = i, attr = (int)TileAttribute.None });
                    added |= uniqueTiles.TryAdd(tileHashV, new() { index = i, attr = (int)TileAttribute.FlipVertical });
                    added |= uniqueTiles.TryAdd(tileHashH, new() { index = i, attr = (int)TileAttribute.FlipHorizontal });
                    added |= uniqueTiles.TryAdd(tileHashVH, new() { index = i, attr = (int)TileAttribute.FlipHorizontalVertical });

                    if (added)
                    {
                        uniqueTileIndices.Add(i);
                    }
                }

                // get list of all sorted unique tiles
                for (int i = 0; i < uniqueTileIndices.Count; i++)
                {
                    TileEntry t = tiles.tiles[uniqueTileIndices[i]];
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
                            TileEntry tileEntry = tiles.tiles[uniqueTileIndices[i]];
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


            // sprite region names
            Parameters regions = GetSubParameters(spriteParameters, "sprite.region");

            List<(string regionName, Rectangle region)> spriteRegions = new();
            foreach (var kv in regions)
            {
                string spriteName = kv.Key;
                string regionString = kv.Value;
                if (TryParseRectString(regionString, out Rect rect))
                {
                    Rectangle region = new(rect.origin.x * 8, rect.origin.y * 8, rect.size.width * 8, rect.size.height * 8);

                    spriteRegions.Add((spriteName, region));
                }
            }

            // output CHR for tiles
            using (AutoStringBuilder auto = AutoStringBuilder.Auto())
            {
                List<TileEntry> tileEntriesToConvert = new();
                foreach (UniqueTileEntry uniqueTileEntry in allUniqueTiles)
                {
                    tileEntriesToConvert.Add(tiles.tiles[uniqueTileEntry.index]);
                }

                // add region names
                foreach ((string regionName, Rectangle region) in spriteRegions)
                {
                    for (int i = 0; i < tileEntriesToConvert.Count; i++)
                    {
                        TileEntry tileEntry = tileEntriesToConvert[i];
                        if (region.Contains(tileEntry.x, tileEntry.y))
                        {
                            tileEntry.tileName = regionName;

                            tileEntriesToConvert[i] = tileEntry;

                            break;
                        }
                    }
                }

                // generate unique tiles
                GenerateTileCHRString(auto.sb, exportName, tileEntriesToConvert);

                // output chr data
                outputChrData.Add(exportName, new()
                {
                    chrRomAsm = auto.sb.ToString(),
                    chrCount = tileEntriesToConvert.Count,
                    is8x16 = false
                });
            }
        }
        #endregion

        #region Font File
        static void ConvertFontFile(string fontFilename, Dictionary<string, ChrRomOutput> outputChrData, in ConvertOptions cmdOptions)
        {
            TryGetFileParamters(fontFilename, string.Empty, out var fontParameters);

            if (!GetBoolParameter(fontParameters, "enable-import", true))
            {
                return;
            }

            string dir = Path.GetDirectoryName(fontFilename);
            string imageFilename = Path.GetFileNameWithoutExtension(fontFilename);

            FileStreamOptions fileOptions = new()
            {
                Mode = FileMode.Open,
                Access = FileAccess.Read
            };

            using var bitmapImage = new ImageBitmap(new FileStream(PathCombine(dir, imageFilename), fileOptions));

            Assert(bitmapImage.Width >= 8, $"Image too small, width: {bitmapImage.Width} requried at least 8px");
            Assert(bitmapImage.Height >= 8, $"Image too small, height: {bitmapImage.Height} requried at least 8px");

            string charMap = GetStringParameter(fontParameters, "charmap");
            Assert(!string.IsNullOrEmpty(charMap), "Font requires 'charmap' property");

            if (!string.IsNullOrEmpty(charMap))
            {
                Rectangle area = new(0, 0, bitmapImage.Width, bitmapImage.Height);
                var tiles = ConvertImageToTiles(bitmapImage, area, 8, 8);

                // output CHR for tiles
                string exportName = NormalizeExportName(Path.GetFileName(fontFilename));
                using (var auto = AutoStringBuilder.Auto())
                {
                    GenerateTileCHRString(auto.sb, exportName, tiles.tiles);

                    outputChrData.Add(exportName, new()
                    {
                        chrRomAsm = auto.sb.ToString(),
                        chrCount = tiles.tiles.Count,
                        is8x16 = false
                    });
                }

                // Generate charmap .s file
                using (var auto = AutoStringBuilder.Auto())
                {
                    var sb = auto.sb;

                    GenerateAsmFileHeader(sb, fontFilename);

                    sb.AppendLine(".feature string_escapes");
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

                    WriteGeneratedAsmFile(sb, fontFilename, in cmdOptions);
                }

                // Generate charmap header file
                using (var auto = AutoStringBuilder.Auto())
                {
                    var sb = auto.sb;

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

                    // wrap contents
                    WrapHeaderFile(sb, fontFilename, fontFilename);

                    // write font file
                    WriteGeneratedHeaderFile(sb, fontFilename, in cmdOptions);
                }
            }
        }
        #endregion

        #region Game Text File
        static void ConvertL10NFile(string l10nFilename, Dictionary<string, ChrRomOutput> outputChrData, in ConvertOptions cmdOptions)
        {
            TryGetFileParamters(l10nFilename, string.Empty, out var l10nParameters);

            string lang = GetStringParameter(l10nParameters, "force-lang", cmdOptions.language);

            string includeRoot = PathCombine(Path.GetDirectoryName(l10nFilename), cmdOptions.defaultGeneratedAssetFolder, "include");
            string[] languageFiles = Directory.GetFiles(includeRoot, $"*.{lang}.*", SearchOption.AllDirectories);

            if (languageFiles.Length > 0)
            {
                using var auto = AutoStringBuilder.Auto();

                foreach (string languageFile in languageFiles)
                {
                    if (languageFile.EndsWith(".h"))
                    {
                        auto.sb.AppendLine($"#include \"{Path.GetFileName(languageFile)}\"");
                    }
                }

                string filename = PathCombine(Path.GetDirectoryName(l10nFilename), Path.GetFileNameWithoutExtension(l10nFilename));

                WrapHeaderFile(auto.sb, l10nFilename, filename);

                WriteGeneratedHeaderFile(auto.sb, filename, in cmdOptions);
            }

        }
        #endregion

        #region Image File
#if false
        static void ConvertImageFile(string imageFilename, Dictionary<string, ChrRomOutput> outputChrData, in ConvertOptions cmdOptions)
        {
            ConvertOptions options = cmdOptions;

            bool isFont = TryGetFileParamters(imageFilename, "font", out var fontParameters);
            bool isBackground = TryGetFileParamters(imageFilename, "background", out var backgroundParameters);

            bool enableImport = GetBoolParameter(spriteParameters, "enable-import", true)
                && GetBoolParameter(fontParameters, "enable-import", true)
                && GetBoolParameter(backgroundParameters, "enable-import", true);

            if (!enableImport)
            {
                return;
            }

            string filename = Path.GetFileNameWithoutExtension(imageFilename);

            FileStreamOptions fileOptions = new()
            {
                Mode = FileMode.Open,
                Access = FileAccess.Read
            };

            using var bitmapImage = new ImageBitmap(new FileStream(imageFilename, fileOptions));

            Assert(bitmapImage.Width >= 8, $"Image too small, width: {bitmapImage.Width} requried at least 8px");
            Assert(bitmapImage.Height >= 8, $"Image too small, height: {bitmapImage.Height} requried at least 8px");

            int metaSpriteWidth = GetIntParameter(spriteParameters, "meta-sprite.width");
            int metaSpriteHeight = GetIntParameter(spriteParameters, "meta-sprite.height");

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
                    added |= uniqueTiles.TryAdd(tileHash, new() { index = i, attr = (int)TileAttribute.None });
                    added |= uniqueTiles.TryAdd(tileHashV, new() { index = i, attr = (int)TileAttribute.FlipVertical });
                    added |= uniqueTiles.TryAdd(tileHashH, new() { index = i, attr = (int)TileAttribute.FlipHorizontal });
                    added |= uniqueTiles.TryAdd(tileHashVH, new() { index = i, attr = (int)TileAttribute.FlipHorizontalVertical });

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

                        sb.AppendLine(".feature string_escapes");
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

                        WrapHeaderFile(sb, fontFilename, fontFilename);

                        // write font file
                        string hFilename = Path.Combine(Path.GetDirectoryName(imageFilename), cmdOptions.defaultGeneratedAssetFolder, "include", $"{filename}.font.h");
                        File.WriteAllText(hFilename, sb.ToString(), UTF8);
                        LogInfo($"Converted {imageFilename} -> {hFilename}");
                    }
                }
            }

        }
#endif
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
            string chrOutputKey = NormalizeExportName($"{Path.GetFileName(asepriteFilename)}.sprite");
            outputChrData.Add(chrOutputKey, new()
            {
                chrRomAsm = sb.ToString(),
                chrCount = 0,
                is8x16 = false,
            });
            #endregion
        }
        #endregion

        #region Memory Utils
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
        #endregion

        private const string kNewLine = "\n";

        #region CSV File
        static void ConvertCSVFile(string textFilename, Dictionary<string, ChrRomOutput> _, in ConvertOptions cmdOptions)
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

                line = line.ReplaceLineEndings(kNewLine);

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

                        line = line.ReplaceLineEndings(kNewLine);

                        // if there are still lines, add a new line to the string buffer
                        if (line != null)
                        {
                            sb.Append(kNewLine);
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

            using var auto = AutoStringBuilder.Auto();
            var sb = auto.sb;

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
                        string[] lines = value.Split(kNewLine, StringSplitOptions.RemoveEmptyEntries);

                        bool allLinesShort = true;
                        foreach (string l in lines)
                        {
                            allLinesShort &= l.Length < kMaxStringWidth;
                        }

                        if (!allLinesShort)
                        {
                            LogError($"Strings {lines.Length} too long [{langToMap.Key}]:[{kv.Key}] -> [{value}]");
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

            // generate each language text files
            foreach (var langToMap in textMap)
            {
                string lang = langToMap.Key;

                sb.Clear();
                sb.AppendLine(";");
                sb.AppendLine($"; Text '{lang}'- Generated from {Path.GetFileName(textFilename)}");
                sb.AppendLine(";");
                sb.AppendLine();


                sb.AppendLine(";");
                sb.AppendLine("; Text Table");
                sb.AppendLine(";");
                sb.AppendLine();

                sb.AppendLine($".segment    \"{kRODATASegment}\"");
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

                sb.AppendLine($".segment    \"{kRODATASegment}\"");
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

                string sFilename = Path.Combine(Path.GetDirectoryName(textFilename), cmdOptions.defaultGeneratedAssetFolder, "asm", $"{filename}.{lang}.s");
                LogInfo($"Converted {textFilename} -> {sFilename}");
                File.WriteAllText(sFilename, sb.ToString(), UTF8);
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

                sb.Clear();

                sb.AppendLine($"#define BUILD_LANGUAGE \"{lang}\"");
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
                    string[] lines = kv.Value.Split(kNewLine, StringSplitOptions.RemoveEmptyEntries);

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
                    string[] lines = kv.Value.Split(kNewLine, StringSplitOptions.RemoveEmptyEntries);

                    int maxWidth = -1;
                    foreach (var line in lines)
                    {
                        maxWidth = Math.Max(maxWidth, line.Length);
                    }
                    Assert(maxWidth > 0);

                    Indent(sb).Append($"{$"{kv.Key}_height",-64} = {lines.Length}").AppendLine(",");
                }

                sb.AppendLine("};");

                string dstFilename = PathCombine(Path.GetDirectoryName(textFilename), $"{filename}.{lang}");
                WrapHeaderFile(sb, textFilename, dstFilename);

                WriteGeneratedHeaderFile(sb, dstFilename, in cmdOptions);
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

        private readonly static string[] ChrRomSegments = {
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

            string exportPath = PathCombine(Path.GetDirectoryName(layoutFilename), Path.GetFileNameWithoutExtension(layoutFilename));

            // write CHR ASM file
            using (var auto = AutoStringBuilder.Auto())
            {
                StringBuilder sb = auto.sb;

                GenerateAsmFileHeader(sb, layoutFilename, chrSegment);

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

                // write output file
                WriteGeneratedAsmFile(sb, exportPath, in cmdOptions);
            }

            // write CHR Header
            using (var auto = AutoStringBuilder.Auto())
            {
                StringBuilder sb = auto.sb;

                sb.Append("enum ").Append(chrSegment).AppendLine();
                sb.AppendLine("{");
                foreach (var chrRom in chrRomLayouts)
                {
                    string key = $"{chrRom.chrRomKey.ToUpperInvariant()} =";
                    sb.AppendLine($"    {key,-64} 0x{chrRom.offset:X2},");
                }
                sb.AppendLine("};");

                string chrHeader = chrSegment.ToLowerInvariant();
                WrapHeaderFile(sb, layoutFilename, chrHeader);

                // write output file
                WriteGeneratedHeaderFile(sb, exportPath, in cmdOptions);
            }
        }
        #endregion

        #region Main App
        private const int LibraryVersion = 2;

        delegate void ConvertFunc(string inputFilename, Dictionary<string, ChrRomOutput> outputChrData, in ConvertOptions convertOptions);

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

        private static int ExitCode = 0;

        private static void PrintHelp() { }

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
                defaultGeneratedAssetFolder = "generated",
                language = "en",
            };

            Dictionary<string, SupportedFormat> formatMap = new()
            {
                { ".csv", new() { convertFunc = ConvertCSVFile, order = 0 } },
                { ".l10n", new () { convertFunc = ConvertL10NFile, order = int.MaxValue} },

                { ".layout", new() {convertFunc = ConvertLayoutFile, order = int.MaxValue } },

                {".sprite", new () {convertFunc = ConvertSpriteFile, order = 0}},
                {".font", new () {convertFunc = ConvertFontFile, order = 0}},
                {".background", new () {convertFunc = ConvertBackgroundImageFile, order = 0}},
                {".palette", new () { convertFunc = ConvertPaletteFile, order = 0}},

                //{ ".aseprite", new(){ convertFunc = ConverteAsepriteFile, order = 0 } },
            };

            List<CompileTask> compilerTasks = new();

            // TODO: force reimport turned on because of new chr rom layout
            //   dependency thing needed to make sure that all sprites are
            //   generated in chr rom when one changes to make sure it's generated correctly
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
                            PrintHelp();
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

                        case "-l":
                        case "--lang":
                            ++i;
                            if (i < args.Length)
                            {
                                options.language = args[i];
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
                        if (!TryGetFileParamters(PathCombine(Path.GetDirectoryName(argPath), ""), "library", out var libraryParameters) || forceReimport)
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
                            WriteParametersToFile(libraryParameters, PathCombine(Path.GetDirectoryName(argPath), ".library"));
                        }
                    }
                }
            }

            HashSet<string> outputGenDirs = new();
            // create output generated folders
            if (compilerTasks.Count > 0)
            {
                foreach (var task in compilerTasks)
                {
                    string outGenDir = PathCombine(Path.GetDirectoryName(compilerTasks[0].inputFileName), options.defaultGeneratedAssetFolder);
                    outputGenDirs.Add(outGenDir);
                }

                foreach (var dir in outputGenDirs)
                {
                    LogInfo($"Output Gen: {dir}");
                    Directory.CreateDirectory(dir);
                    Directory.CreateDirectory(PathCombine(dir, "asm"));
                    Directory.CreateDirectory(PathCombine(dir, "include"));
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
        #endregion
    }
}

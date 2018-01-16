using System;
using System.Collections.Generic;
using System.Linq;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices; 
using Btfax.CommonLib;
using Btfax.CommonLib.Util;

namespace TiffProcessor.TIFProcessing
{
    class PROCESSING_INFO
    {
        public bool exist = false;
        public decimal seq = -1;
        public string strFile = "";
        public string strProcessingMode = "";
        public string strPages = "";
        public bool processed = false;
    };

    class TifAccess
    {
        #region static
        static ImageCodecInfo codecInfo_TIF = null;
        static EncoderParameter ep_CCITT3 = new EncoderParameter(Encoder.Compression, (long)EncoderValue.CompressionCCITT3);
        static EncoderParameter ep_MultiFrame = new EncoderParameter(Encoder.SaveFlag, (long)EncoderValue.MultiFrame);
        static EncoderParameter ep_FramePage = new EncoderParameter(Encoder.SaveFlag, (long)EncoderValue.FrameDimensionPage);
        static EncoderParameter ep_Flush = new EncoderParameter(Encoder.SaveFlag, (long)EncoderValue.Flush);

        static EncoderParameters eps_Frame = new EncoderParameters(2);
        static EncoderParameters eps_FramePage = new EncoderParameters(2);
        static EncoderParameters eps_Flush = new EncoderParameters(1);

        static TifAccess()
        {
            ImageCodecInfo[] encoders;
            encoders = ImageCodecInfo.GetImageEncoders();
            foreach (ImageCodecInfo encoder in encoders)
            {
                if (encoder.MimeType != "image/tiff")
                    continue;

                codecInfo_TIF = encoder;
                break;
            }

            TifAccess.eps_Frame.Param[0] = TifAccess.ep_MultiFrame;
            TifAccess.eps_Frame.Param[1] = TifAccess.ep_CCITT3;
            TifAccess.eps_FramePage.Param[0] = TifAccess.ep_FramePage;
            TifAccess.eps_FramePage.Param[1] = TifAccess.ep_CCITT3;
            TifAccess.eps_Flush.Param[0] = TifAccess.ep_Flush;
        }
        #endregion

        #region constructor
        public TifAccess(){ }
        #endregion

        #region method
        public RESULT Merge(string p_strDestFile, ref List<PROCESSING_INFO> p_fileProcessInfos)
        {
            try
            {
                bool frameBuilded = false;
                Bitmap bitmap_src = null;
                Bitmap bitmap_dst = null;

                bitmap_dst = null;
                foreach (PROCESSING_INFO info in p_fileProcessInfos)
                {
                    if (!info.exist)
                        continue;

                    string strPages = "";
                    if (info.strProcessingMode != ((int)DOC_PROCESSING_MODE.TPC_EXTRACT).ToString() )
                        strPages = "all";
                    else
                    {
                        strPages = (info.strPages == "all" || info.strPages.Length <= 0) ? "all" : string.Format(",{0},", info.strPages);
                    }

                    try
                    {
                        bitmap_src = new Bitmap(info.strFile);
                    }
                    catch { return RESULT.F_FILE_NOT_EXIST; }

                    int pageCnt = bitmap_src.GetFrameCount(FrameDimension.Page);


                    // 김창기 씨, 아래가 필요한 이유는?
                    ////// 추출페이지 유효성 검증
                    if (strPages != null && strPages != "all")
                    {
                        string[] strExtractPages = info.strPages.Split(new char[] { ',' });
                        foreach (string strExtractPg in strExtractPages)
                        {
                            int extractPg;
                            try { extractPg = Convert.ToInt32(strExtractPg); }
                            catch { return RESULT.F_SYSTEMHEADER_DOCFILELIST_FORMAT_INVALID; }

                            if (extractPg > pageCnt)
                                return RESULT.F_MAKE_IMAGEFILE_PAGECOUNT_NOT_EXIST;
                        }
                    }

                    //// 페이지 추출
                    //bool bitmap_src_temporary = false;
                    for (int pg = 0; pg < pageCnt; pg++)
                    {
                        if (strPages != "all" && strPages.IndexOf(string.Format(",{0},", pg + 1)) < 0)
                            continue;

                        bitmap_src.SelectActiveFrame(FrameDimension.Page, pg);


                        #region ADD - KIMCG : 20111206
                        //// 이미지 Width 가 1728 이 아니면 다시 그린다
                        Bitmap newBitmap = null;
                        if (bitmap_src.Width != 1728)
                        {
                            newBitmap = ReDrawToA4Size(bitmap_src);
                            newBitmap = BitmapTo1Bpp(newBitmap);
                        }
                        #endregion

                        if (!frameBuilded)
                        {
                            frameBuilded = true;

                            if (newBitmap != null)
                                bitmap_dst = newBitmap;
                            else
                                bitmap_dst = bitmap_src;

                            bitmap_dst.Save(p_strDestFile, TifAccess.codecInfo_TIF, TifAccess.eps_Frame);
                        }
                        else
                        {
                            if (bitmap_dst != null)
                            {
                                if (newBitmap != null)
                                    bitmap_dst.SaveAdd(newBitmap, TifAccess.eps_FramePage);
                                else
                                    bitmap_dst.SaveAdd(bitmap_src, TifAccess.eps_FramePage);
                            }
                        }

                        if (newBitmap != null)
                            newBitmap.Dispose();
                    }

                    info.processed = true;
                }

                if (bitmap_dst != null)
                    bitmap_dst.SaveAdd(eps_Flush);

                bitmap_src.Dispose();
                bitmap_dst.Dispose();

                return RESULT.SUCCESS;
            }
            catch (Exception ex)
            {
                Program.LogAndDisplay(Btfax.CommonLib.Log.LOG_LEVEL.ERR, ex.Message);
                return RESULT.F_TIFFPROCESS_FAIL_TO_MERG;
            }
        }

        #region ADD - KIMCG : 20111206
        private static Bitmap ReDrawToA4Size(Bitmap p_srcBmp)
        {
            //// width 값이 변경된 길이 비율만큼 세로 길이를 맞춤
            int w = 1728;
            int h = 2295;
            int nVal = (1728 / p_srcBmp.Width);
            if (nVal == 0)
                nVal = 1;

            Bitmap dstBmp = null;
            Graphics g = null;
            try
            {
                dstBmp = new Bitmap(w, h);
                dstBmp.SetResolution(196, 196);
                g = Graphics.FromImage(dstBmp);
                g.FillRectangle(new SolidBrush(Color.White), new Rectangle(0, 0, w, h));
                //g.DrawImage(p_srcBmp, new Rectangle((w - ((w - p_srcBmp.Width) * nVal)),
                //                                    h - ((h - p_srcBmp.Height) * nVal), 
                //                                    p_srcBmp.Width * nVal, 
                //                                    p_srcBmp.Height * nVal));

                g.DrawImage(p_srcBmp, new Rectangle(nVal * 10,
                                                    h - ((h - p_srcBmp.Height * nVal)),
                                                    p_srcBmp.Width * nVal,
                                                    p_srcBmp.Height * nVal));
                
            }
            finally
            {
                g.Dispose();
            }
            return dstBmp;
        }

        // ADD - KJC : 20110210
        private static Bitmap BitmapTo1Bpp(Bitmap bmp_src)
        {
            int w = bmp_src.Width;
            int h = bmp_src.Height;
            
            BitmapData bmpData_src = bmp_src.LockBits(new Rectangle(0, 0, w, h), ImageLockMode.ReadWrite, PixelFormat.Format32bppArgb);
            int stride_src = bmpData_src.Stride;
            int[] data_src = new int[stride_src];

            Bitmap bmp_dest = new Bitmap(w, h, PixelFormat.Format1bppIndexed);
            BitmapData bmpData_dest = bmp_dest.LockBits(new Rectangle(0, 0, w, h), ImageLockMode.ReadWrite, PixelFormat.Format1bppIndexed);
            int stride_dest = bmpData_dest.Stride;
            byte[] data_dest = new byte[stride_dest];

            for (int y = 0; y < h; y++)
            {
                if (y < h -3)
                {
                    Marshal.Copy((IntPtr)((long)bmpData_src.Scan0 + bmpData_src.Stride * y), data_src, 0, data_src.Length);
                    for (int x = 0; x < stride_src; x++)
                    {
                        if (x / 8 >= stride_dest)
                            break;

                        if (x % 8 == 0)
                            data_dest[x / 8] = 0;

                        //if (((UInt32)data_src[x]) >= 0xFF7F7F7F)
                        if (((UInt32)data_src[x]) >= 0xFF7F7F82)
                            data_dest[x / 8] |= (byte)(0x80 >> (x % 8));
                    }
                    Marshal.Copy(data_dest, 0, (IntPtr)((long)bmpData_dest.Scan0 + bmpData_dest.Stride * y), data_dest.Length);
                }
                else
                {
                    for (int x = 0; x < stride_src; x++)
                    {
                        if (x / 8 >= stride_dest)
                            break;
                        #region MODIFY - KCG : 20110804
                        //if (x % 8 == 0)
                        //    data_dest[x / 8] = 0;

                        //if (((UInt32)data_src[x]) >= 0xFF7F7F82)
                        if (((UInt32)data_src[x]) >= 0xFF7F7F7F)
                            data_dest[x / 8] |= (byte)(0x80 >> (x % 8));
                        #endregion
                    }
                    Marshal.Copy(data_dest, 0, (IntPtr)((long)bmpData_dest.Scan0 + bmpData_dest.Stride * y), data_dest.Length);
                }
            }

            bmp_src.UnlockBits(bmpData_src);
            bmp_dest.UnlockBits(bmpData_dest);

            bmp_dest.SetResolution(204, 196);
            return bmp_dest;
        }
        #endregion
        #endregion

    }
}

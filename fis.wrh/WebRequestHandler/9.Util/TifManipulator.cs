using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Threading;
using Btfax.CommonLib.Log;

namespace WebRequestHandler
{
    public static class TifManipulator
    {
        #region constructor
        static TifManipulator()
        {
            ImageCodecInfo[] encoders;
            encoders = ImageCodecInfo.GetImageEncoders();
            foreach (ImageCodecInfo encoder in encoders)
            {
                if (encoder.MimeType != "image/tiff")
                    continue;

                CODEC_TIF = encoder;
                break;
            }

            EPS_Frame.Param[0] = EP_MultiFrame;
            EPS_Frame.Param[1] = EP_CCITT3;
            EPS_FramePage.Param[0] = EP_FramePage;
            EPS_FramePage.Param[1] = EP_CCITT3;
            EPS_Flush.Param[0] = EP_Flush;
        }
        #endregion

        #region method
        // 페이지수 다르게 SPLIT
        //      파라미터 : 원본 파일 이름
        public static bool SplitTif(string p_strPath, string p_strSrcFile, string p_strPages, string p_strOutFilePrefix, ref List<DbModule.SPLIT_FILE_INFO> p_lstOutFiles)
        {
            bool bResult;
            Bitmap srcFile = null;

            bResult = true;
            try
            {
                srcFile = new Bitmap(p_strPath + '/' + p_strSrcFile);

                if (!SplitTif(p_strPath, srcFile, p_strPages, p_strOutFilePrefix, ref p_lstOutFiles))
                    bResult = false;
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, "TIF파일 분리 시, 예외가 발생하였습니다.");
                bResult = false;
            }

            try 
            { 
                if (srcFile != null) 
                    srcFile.Dispose(); 
            }
            catch { }

            return bResult;
        }

        // 페이지수 다르게 SPLIT
        //      파라미터 : 원본 파일 비트맵 객체
        public static bool SplitTif(string p_strPath, Bitmap p_srcFile, string p_strPages, string p_strOutFilePrefix, ref List<DbModule.SPLIT_FILE_INFO> p_lstOutFiles)
        {
            int     from, to;
            char[] dash = new char[] { '-' };
            DbModule.SPLIT_FILE_INFO splitFileInfo = new DbModule.SPLIT_FILE_INFO();
            
            try
            {
                string[] pages = p_strPages.Split(new char[] { ',' });

                // SPLIT 루프
                for (int i = 0; i < pages.Length; ++i)
                {
                    // SPLIT 파일 정보 SET
                    splitFileInfo.Clear();
                    splitFileInfo.p_strPagesInOrg = pages[i];
                    splitFileInfo.p_strFile = string.Format("{0}_{1}.tif", p_strOutFilePrefix, i + 1);

                    // SPLIT 파일 : from - to 얻기
                    string[] fromTo = pages[i].Split(dash);

                    if (fromTo.Length == 1)
                    {
                        from = Convert.ToInt32(fromTo[0]);
                        to   = from;;
                    }
                    else if (fromTo.Length == 2) // 예1) "3-5", 예2) "7-"
                    {
                        from = Convert.ToInt32(fromTo[0]);
                        if (fromTo[1].Length > 0)
                            to = Convert.ToInt32(fromTo[1]);                    // 예1) "3-5"
                        else
                            to = p_srcFile.GetFrameCount(FrameDimension.Page);  // 예2) "7-". 끝페이지까지
                    }
                    else
                    {
                        AppLog.Write(LOG_LEVEL.ERR, string.Format("TIF파일 분리 시, 분리 페이지 정보가 잘 못 되었습니다.({0})", p_strPages));
                        return false;
                    }

                    // SPLIT
                    SplitTif(p_strPath, p_srcFile, from, to, ref splitFileInfo);
                    
                    // SPLIT 파일 정보 추가
                    p_lstOutFiles.Add(splitFileInfo);
                    
                }
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, "TIF파일 분리 시, 예외가 발생하였습니다.");
                return false;
            }

            return true;
        }

        // 페이지수 동일 SPLIT
        public static bool SplitTif(string p_strPath, string p_strSrcFile, int p_nEachPage, string p_strOutFilePrefix, ref List<DbModule.SPLIT_FILE_INFO> p_lstOutFiles)
        {
            bool    bResult;
            Bitmap  srcFile = null;
            string  strPages = "";
            
            if (p_nEachPage <= 0)
                return false;

            bResult = true;
            try
            {
                srcFile = new Bitmap(p_strPath + '/' + p_strSrcFile);

                int nPageCnt = srcFile.GetFrameCount(FrameDimension.Page);
                if (nPageCnt <= 0)
                    bResult = false;

                // 추출 페이지 포맷 만들기
                if (bResult)
                {
                    for (int i = 1; i <= nPageCnt; i += p_nEachPage)
                    {
                        if (p_nEachPage == 1)
                        {
                            strPages += string.Format( "{0},", i );
                        }
                        else
                        {
                            int nPageBegin = i;
                            int nPageEnd = i + p_nEachPage - 1;

                            if (nPageEnd <= nPageCnt)
                                strPages += string.Format("{0}-{1}", i, i + p_nEachPage - 1) + ',';
                            else
                                strPages += string.Format("{0}-{1}", i, nPageCnt) + ',';
                        }
                    }
                    strPages = strPages.Remove(strPages.Length - 1);
                }

                // 추출
                if (bResult)
                {
                    if (!SplitTif(p_strPath, srcFile, strPages, p_strOutFilePrefix, ref p_lstOutFiles))
                        bResult = false;
                }
            }
            catch (Exception ex)
            {
                AppLog.ExceptionLog(ex, "TIF파일 분리 시, 예외가 발생하였습니다.");
                bResult = false;
            }

            try 
            { 
                if (srcFile != null) 
                    srcFile.Dispose(); 
            }
            catch { }

            return bResult;
        }

        #endregion

        #region implementation
        private static bool SplitTif(string p_strPath, Bitmap p_srcFile, int p_nPageBegin, int p_nPageEnd, ref DbModule.SPLIT_FILE_INFO p_fileInfo)
        {
            Bitmap  outFile = null;
            Bitmap  outTempFile = null;
            Bitmap  newFile;
            bool    bFrameBuild;
            string  strPathFile = p_strPath + "\\" + p_fileInfo.p_strFile;
            
            
            // TIF 페이지 추출 생성
            bFrameBuild = false;
            for (int i = p_nPageBegin-1 ; i <= p_nPageEnd-1; ++i)
            {
                p_srcFile.SelectActiveFrame(FrameDimension.Page, i);

                // TIF 가로 픽셀이 1728이 아닌경우, Bitmap 재생성
                newFile = null;
                if (p_srcFile.Width == 1728)
                {
                    outTempFile = p_srcFile;
                }
                else
                {
                    newFile = ReDrawToA4Size(p_srcFile);
                    newFile = BitmapTo1Bpp(newFile);
                    outTempFile = newFile;
                }
                

                if (!bFrameBuild)
                {
                    bFrameBuild = true;

                    outFile = outTempFile;
                    outFile.Save(strPathFile, CODEC_TIF, EPS_Frame);
                }
                else
                {
                    outFile.SaveAdd(outTempFile, EPS_FramePage);
                }

                if (newFile != null)
                    newFile.Dispose();
            }

            outFile.SaveAdd(EPS_Flush);

            // SPLIT 파일 정보 SET
            FileInfo info = new FileInfo(strPathFile);
            p_fileInfo.p_nPageCnt = p_nPageEnd - p_nPageBegin + 1;
            p_fileInfo.p_fileSize = info.Length;

            return true;
        }

        private static Bitmap ReDrawToA4Size(Bitmap p_srcBmp)
        {
            // width 값이 변경된 길이 비율만큼 세로 길이를 맞춤
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
                if (y < h - 3)
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

        #region field
        static ImageCodecInfo CODEC_TIF = null;
        static EncoderParameter EP_CCITT3 = new EncoderParameter(Encoder.Compression, (long)EncoderValue.CompressionCCITT3);
        static EncoderParameter EP_MultiFrame = new EncoderParameter(Encoder.SaveFlag, (long)EncoderValue.MultiFrame);
        static EncoderParameter EP_FramePage = new EncoderParameter(Encoder.SaveFlag, (long)EncoderValue.FrameDimensionPage);
        static EncoderParameter EP_Flush = new EncoderParameter(Encoder.SaveFlag, (long)EncoderValue.Flush);

        static EncoderParameters EPS_Frame = new EncoderParameters(2);
        static EncoderParameters EPS_FramePage = new EncoderParameters(2);
        static EncoderParameters EPS_Flush = new EncoderParameters(1);
        #endregion
    }
}

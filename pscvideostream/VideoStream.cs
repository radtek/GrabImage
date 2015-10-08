using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

namespace pscvideostream
{
    namespace grabimage
    {
        class MyClass
        {
            [DllImport("GrabImage.dll", CharSet = CharSet.Auto)]
            public static extern void StartPreview(IntPtr handle, int width, int height);

            //[DllImport("GrabImage.dll", CharSet = CharSet.Auto)]
            //public static extern long MakeOneShot();

            [DllImport("GrabImage.dll", CharSet = CharSet.Ansi)]
            public static extern void TakeSnap(String fileName);

            [DllImport("GrabImage.dll", CharSet = CharSet.Auto)]
            public static extern void DestroyGraph();
        }
    }

    public class VideoStream
    {
        bool protect = false;

        public string ErrorMessage { get; set; }

        public VideoStream()
        {
        }

        public int StartPreview(IntPtr hWnd, int width, int height)
        {
            if (!protect)
            {
                try
                {
                    grabimage.MyClass.StartPreview(hWnd, width, height);
                }
                catch (Exception ex)
                {
                    ErrorMessage = ex.Message;
                    //ErrorMessage = "Error - No camera attached";
                    return 1;
                }
            }
            return 0;
        }

        public byte[] TakeSnap(long quality, string fileName)
        {
            if (protect)
                return null;

            FileStream fs = null;
            MemoryStream ms = null;
            Image img = null;
            Bitmap bmp = null;
            byte[] bmpArray = null;
            byte[] jpgArray = null;
            try
            {
                grabimage.MyClass.TakeSnap(fileName);

                fs = new FileStream(fileName, FileMode.Open);
                BinaryReader br = new BinaryReader(fs);
                bmpArray = br.ReadBytes((int)fs.Length);

                img = Image.FromStream(fs);

                //ms = new MemoryStream(buffer);
                //img = Image.FromStream(ms);
                //ms.Dispose();

                ms = new MemoryStream();
                bmp = new Bitmap(img);
                saveJpegToStream(out ms, bmp, quality);

                jpgArray = ms.ToArray();
            }
            catch (Exception ex)
            {
                ErrorMessage = ex.Message;
                return null;
            }
            finally
            {
                fs.Dispose();
                ms.Dispose();
                bmp.Dispose();
                img.Dispose();
                //File.Delete(fileName);
            }

            return jpgArray;
        }

        public void DestroyGraph()
        {
            if (!protect)
            {
                try
                {
                    grabimage.MyClass.DestroyGraph();
                }
                catch {}
            }
        }

        private void saveJpegToStream(out MemoryStream stream, Bitmap img, long quality)
        {
            stream = new MemoryStream();
            // Encoder parameter for image quality
            EncoderParameter qualityParam = new EncoderParameter(System.Drawing.Imaging.Encoder.Quality, quality);

            // Jpeg image codec
            ImageCodecInfo jpegCodec = getEncoderInfo("image/jpeg");

            if (jpegCodec == null)
                return;

            EncoderParameters encoderParams = new EncoderParameters(1);
            encoderParams.Param[0] = qualityParam;

            img.Save(stream, jpegCodec, encoderParams);
        }

        private ImageCodecInfo getEncoderInfo(string mimeType)
        {
            // Get image codecs for all image formats
            ImageCodecInfo[] codecs = ImageCodecInfo.GetImageEncoders();

            // Find the correct image codec
            for (int i = 0; i < codecs.Length; i++)
                if (codecs[i].MimeType == mimeType)
                    return codecs[i];

            return null;
        }

    }
}

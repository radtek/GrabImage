using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using System.Runtime.InteropServices;
using System.IO;

namespace GrabImageClient
{
    enum SAVE
    {
        INSERT = 0,
        UPDATE = 1
    }

    public partial class Form1 : Form
    {
        [DllImport("GrabImage.dll", CharSet = CharSet.Auto)]
        public static extern int StartPreview(IntPtr handle, int width, int height);

        [DllImport("GrabImage.dll", CharSet = CharSet.Auto)]
        public static extern long MakeOneShot();

        [DllImport("GrabImage.dll", CharSet = CharSet.Ansi)]
        public static extern int TakeSnap(String fileName);
        //        public static extern void TakeSnap(System.Text.StringBuilder fileName);
        //        public static extern void TakeSnap(char[] fileName);

        [DllImport("GrabImage.dll", CharSet = CharSet.Auto)]
        public static extern void DestroyGraph();

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            //DBUtil db = new DBUtil();
            //db.UpgrateSQLServerCe();
            //return;

            IntPtr hWnd = pictureBox1.Handle;
            if (StartPreview(hWnd, pictureBox1.Width, pictureBox1.Height) == 1)
                button1.Enabled = false;
            else
                button1.Focus();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            //long length = MakeOneShot();

            //System.Text.StringBuilder fileName = new System.Text.StringBuilder();
            //fileName.Append("EMP_PICS.bmp");
            //            fileName.Append("c:\\temp\\EMP_PICS.bmp");

            //char[] fileName = "c:\\temp\\EMP_PICS.bmp".ToCharArray();

            //            String fileName = "c:\\temp\\EMP_PICS.bmp";
            String fileName = "EMP_PICS.bmp";


            if (TakeSnap(fileName) == 1)
                return;

            //IntPtr hWnd = pictureBox1.Handle;
            //StartPreview(hWnd, pictureBox1.Width, pictureBox1.Height);

            FileStream fs = null;
            MemoryStream ms = null;
            try
            {
                fs = new FileStream(fileName, FileMode.Open);
                BinaryReader br = new BinaryReader(fs);
                byte[] buffer = br.ReadBytes((int)fs.Length);
                fs.Close();
                //Image img = Image.FromStream(fs);

                ms = new MemoryStream(buffer);
                Image img = Image.FromStream(ms);
            
                pictureBox2.Image = img;

                //img.Save("c:\\aaa.jpg", ImageFormat.Jpeg);

                //MemoryStream mem = new MemoryStream();
                //img.Save(mem, System.Drawing.Imaging.ImageFormat.Jpeg);
                //byte[] buff = mem.ToArray();

                button1.Focus();

                ms.Close();

                //Bitmap bmp = new Bitmap(img);
                //Helper.saveJpegToStream(out ms, bmp, 50L);
                //bmp.Dispose();
                //img = Image.FromStream(ms);
                //img.Save("aaa50quality.jpg", ImageFormat.Jpeg);

                //Helper.saveJpeg("EMP_PICS.jpg", new Bitmap(img), 100L);

                DBUtil db = new DBUtil();
                db.SavePicture((int)SAVE.INSERT, 6, ref buffer);
                buffer = db.GetPicure(1);
            }
            catch (Exception ex) {
                MessageBox.Show(ex.Message);
            }
            finally
            {
                fs.Close();
                ms.Close();
                //File.Delete(fileName);
            }
        }
    }
}

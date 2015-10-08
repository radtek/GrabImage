using System;
using System.Text;
using System.Security.Cryptography;
using System.IO;

namespace pscvideostream
{
    //class member variable
    //bool protect = true;
    
    // Put the following code in a protected class constructor
    //Cryptography.FirstTimeSaveDigestInFile(1);


    //if (Cryptography.TestProtection())
    //    protect = true;
    //else
    //    protect = false;


    class Cryptography
    {
        //!This program cannot be run in DOS mode.

        static string _filename = "rid50.dll";
        static byte[] _fileContent = null;
        static int position = 350;
        static int digestLength = 20;

        public static void FirstTimeSaveDigestInFile(int i)
        {
            //string istr = GetDigestAsString(Convert.ToString(i));
            //SaveDigestInFile(istr);
            byte[] digest = GetDigestAsBytes(Convert.ToString(i));
            _fileContent = ReadDigestFromFile();
            SaveDigestInFile(digest);
        }

        public static bool TestProtection()
        {
            if (!File.Exists(_filename)) { _fileContent = null; return true; };

            _fileContent = ReadDigestFromFile();

            if (_fileContent[position] != 0x13 || _fileContent[position + digestLength + 1] != 0x23)
            {
                _fileContent = null;
                return true;
            };

            byte[] digest = new byte[digestLength];
            Buffer.BlockCopy(_fileContent, position + 1, digest, 0, digestLength);

            /*
                        System.Text.ASCIIEncoding ae = new System.Text.ASCIIEncoding();
                        char[] chars = new char[ae.GetCharCount(buffer)];
                        chars = ae.GetChars(buffer);
                        string digest = new string(chars, 2, chars.Length - 2);
            */
            int threshold = 642;
            byte[] thresholdDigest = GetDigestAsBytes(Convert.ToString(threshold));
            //string thresholdDigest = GetDigestAsString(Convert.ToString(threshold));

            //if (digest == thresholdDigest)
            if (Compare(digest, thresholdDigest))
            {
                _fileContent = null;
                return true;
            }
            else
            {
                for (int i = 1; i < threshold; i++)
                {
                    //string idigest = GetDigestAsString(Convert.ToString(i));
                    byte[] idigest = GetDigestAsBytes(Convert.ToString(i));
                    //if (idigest == digest)
                    if (Compare(digest, idigest))
                    {
                        //idigest = GetDigestAsString(Convert.ToString(i + 1));
                        idigest = GetDigestAsBytes(Convert.ToString(i + 1));
                        SaveDigestInFile(idigest);
                        break;
                    }
                }

                _fileContent = null;
                return false;
            }
        }

        public static bool Compare(byte[] b1, byte[] b2)
        {
            if (b1.Length != b2.Length)
                return false;
            for (int i = 0; i < b1.Length; ++i)
                if (b1[i] != b2[i])
                    return false;
            return true;
        }

        private static byte[] ReadDigestFromFile()
        {
            FileStream fs = new FileStream(_filename, FileMode.Open);
            BinaryReader br = new BinaryReader(fs);
            byte[] buffer = br.ReadBytes((int)fs.Length);
            br.Close();
            fs.Dispose();

            return buffer;
        }

        private static void SaveDigestInFile(string digest)
        {
            //System.Reflection.Assembly assembly = System.Reflection.Assembly.GetExecutingAssembly();
            //string file_path = Path.GetDirectoryName(assembly.Location);
            //if (File.Exists(filename)) { File.Delete(filename); };

            digest = "MZ" + digest;
            FileStream fs = new FileStream(_filename, FileMode.Create);
            BinaryWriter bw = new BinaryWriter(fs);
            System.Text.ASCIIEncoding ae = new System.Text.ASCIIEncoding();
            bw.Write(ae.GetBytes(digest));
            bw.Close();
            fs.Dispose();
        }

        private static void SaveDigestInFile(byte[] digest)
        {
            FileStream fs = new FileStream(_filename, FileMode.Open);
            BinaryWriter bw = new BinaryWriter(fs);
            //System.Text.ASCIIEncoding ae = new System.Text.ASCIIEncoding();
            //bw.Write(ae.GetBytes("MZ"));
            bw.Write(_fileContent, 0, position);
            bw.Write(new byte[1] { 0x13 });
            bw.Write(digest);
            bw.Write(new byte[1] { 0x23 });
            bw.Write(_fileContent, position + digest.Length + 2, _fileContent.Length - position - (digest.Length + 2));
            bw.Close();
            fs.Dispose();
        }

        private static byte[] GetDigestAsBytes(string message)
        {
            SHA1 sha = new SHA1Managed();
            ASCIIEncoding ae = new ASCIIEncoding();
            byte[] data = ae.GetBytes(message);
            byte[] digest = sha.ComputeHash(data);

            return digest;
        }
        private static string GetDigestAsString(string message)
        {
            SHA1 sha = new SHA1Managed();
            ASCIIEncoding ae = new ASCIIEncoding();
            byte[] data = ae.GetBytes(message);
            byte[] digest = sha.ComputeHash(data);

            return GetAsHexaDecimal(digest);
        }

        private static string GetAsString(byte[] bytes)
        {
            StringBuilder s = new StringBuilder();

            int length = bytes.Length;
            for (int n = 0; n < length; n++)
            {
                s.Append((int)bytes[n]);
                if (n != length - 1) { s.Append(' '); }
            }

            return s.ToString();
        }

        private static string GetAsHexaDecimal(byte[] bytes)
        {
            StringBuilder s = new StringBuilder();
            int length = bytes.Length;

            for (int n = 0; n < length; n++)
            {
                s.Append(String.Format("{0,2:x}", bytes[n]).Replace(" ", "0"));
            }

            //System.Text.ASCIIEncoding ae = new System.Text.ASCIIEncoding();
            //return ae.GetBytes(s.ToString());

            return s.ToString();
        }
    }
}

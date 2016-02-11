using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Configuration;
//using System.Data.SqlClient;
using System.Data;
using System.Data.SqlTypes;
using System.Data.SqlServerCe;
using System.Data.Common;
using System.Data.SqlClient;

namespace GrabImageClient
{
    class DBUtil
    {
        internal byte[] GetPicure(int id)
        {
            SqlCeConnection conn = null;
            SqlCeCommand cmd = null;
            SqlCeDataReader reader = null;

            byte[] buffer = null;

            try
            {
                conn = new SqlCeConnection(getConnectionString());
                conn.Open();

                cmd = new SqlCeCommand();
                cmd.Connection = conn;

                cmd.CommandText = "SELECT id, picture FROM images WHERE id = @id";

                //cmd.Parameters.Add(new SqlCeParameter("@id", SqlDbType.Int));   // doesn't work
                cmd.Parameters.AddWithValue("@id", id);
                
                //cmd.Parameters.Add("@id", SqlDbType.Int);
                //cmd.Parameters[0].Value = id;

                reader = cmd.ExecuteReader();
                //reader.Read();

                //SqlBinary binary;
                //SqlBytes bytes;

//                if (reader.HasRows)   //Does not work for CE
                if (reader.Read())
                {
                    if (!reader.IsDBNull(0))
                        id = reader.GetInt32(0);
                    if (!reader.IsDBNull(1))
                    {
                        //binary = reader.GetSqlBinary(1);
                        buffer = (byte[])reader["picture"];

                        //int maxSize = 200000;
                        //buffer = new byte[maxSize];
                        //reader.GetBytes(1, 0L, buffer, 0, maxSize);
                    }
                }
            }
            catch (Exception ex)
            {
                throw new Exception(ex.Message);
            }
            finally
            {
                try
                {
                    if (reader != null)
                        reader.Close();

                    if (conn.State == ConnectionState.Open)
                        conn.Close();

                    if (conn != null)
                        conn = null;
                }
                catch (Exception ex)
                {
                    throw new Exception(ex.Message);
                }
            }

            return buffer;

        }

        internal void SavePicture(int mode, int id, ref byte[] buffer)
        {
            SqlCeConnection conn = null;
            SqlCeCommand cmd = null;

            try
            {
                conn = new SqlCeConnection(getConnectionString());
                conn.Open();

                cmd = new SqlCeCommand();
                cmd.Connection = conn;

                if (mode == (int)SAVE.UPDATE)
                    cmd.CommandText = "UPDATE images SET picture = @picture WHERE id = @id";
                else
                    cmd.CommandText = "INSERT INTO images (id, picture) VALUES (@id, @picture)";

                cmd.Parameters.Add("@picture", SqlDbType.Image);
                cmd.Parameters["@picture"].Value = buffer;

                cmd.Parameters.Add("@id", System.Data.SqlDbType.Int);
                cmd.Parameters["@id"].Value = id;

                cmd.ExecuteNonQuery();
            }
            catch (Exception ex)
            {
                throw new Exception(ex.Message);
            }
            finally
            {
                try
                {
                    if (conn.State == ConnectionState.Open)
                        conn.Close();

                    if (conn != null)
                        conn = null;
                }
                catch (Exception ex)
                {
                    throw new Exception(ex.Message);
                }
            }
        }

        public void UpgrateSQLServerCe()
        {
            SqlCeEngine engine = new SqlCeEngine(getConnectionString());

            // https://msdn.microsoft.com/en-us/library/bb896160.aspx
                 
            engine.Upgrade(getConnectionString());
        }

        private String getConnectionString()
        {
            string connectionString = "GrabImageClient.Properties.Settings.GrabImageConnectionString";
                    //connectionString = "GrabImageClient.Properties.Settings.petitionersConnectionString";

                    return ConfigurationManager.ConnectionStrings[connectionString].ToString();
        }
    }
}

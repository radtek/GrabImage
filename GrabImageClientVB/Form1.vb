Imports pscvideostream

Public Class Form1
    Dim _vs As VideoStream = Nothing


    Private Sub button1_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles button1.Click
        Dim img As Image = Nothing
        Dim ms As IO.MemoryStream = Nothing
        Try
            Dim fileName As String = "EMP_PICS.jpg"

            'Dim jpgByteArray As Byte() = Nothing
            Dim jpgByteArray As Byte() = _vs.TakeSnap(50L, fileName)   ' 50L - quality

            If jpgByteArray IsNot Nothing Then
                ms = New IO.MemoryStream(jpgByteArray)
                img = Image.FromStream(ms)

                PictureBox2.Image = img
            End If

            'Dim hWnd As IntPtr = PictureBox1.Handle
            '_vs.StartPreview(hWnd, PictureBox1.Width, PictureBox1.Height)

        Catch
        Finally
        End Try
    End Sub

    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        Dim hWnd As IntPtr = PictureBox1.Handle
        Dim errorCode As Int32

        _vs = New VideoStream
        errorCode = _vs.StartPreview(hWnd, PictureBox1.Width, PictureBox1.Height)
        If (errorCode <> 0) Then
            toolStripStatusLabelError.Text = _vs.ErrorMessage
            toolStripStatusLabelError.ForeColor = Color.Red
            button1.Enabled = False
            Application.DoEvents()
        Else
            button1.Focus()
        End If

    End Sub

    Private Sub Form1_FormClosed(ByVal sender As System.Object, ByVal e As System.Windows.Forms.FormClosedEventArgs) Handles MyBase.FormClosed
        _vs.DestroyGraph()
    End Sub
End Class

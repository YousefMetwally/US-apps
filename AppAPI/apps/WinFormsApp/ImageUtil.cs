using System;
using System.Diagnostics;
using System.Drawing;
using System.Numerics;
using AppAPI;

class ImageUtil
{
    static Vector3 FromRawArray (float[] arr)
    {
        Debug.Assert(arr.Length == 3);

        return new Vector3(arr[0], arr[1], arr[2]);
    }

    static float[] ToRawArray(Vector3 vec)
    {
        return new float[3] { vec.X, vec.Y, vec.Z };
    }

    /** Grow a "view" to match the aspect ratio of the on-screen resolution. */
    public static Cart2dGeomData GrowViewToFitAspectRatio(Cart2dGeomData view, uint[] resolution)
    {
        float screen_aspect = resolution[0]*1.0f/resolution[1]; // >1 mean wide; <1 means narrow

        Vector3 origin = FromRawArray(view.origin);
        Vector3 dir1 = FromRawArray(view.dir1);
        Vector3 dir2 = FromRawArray(view.dir2);

        if (dir1.Length()/dir2.Length() < screen_aspect)
        {
            // grow view along len1
            float new_len1 = screen_aspect*dir2.Length();

            // shift origin
            origin -= dir1*(new_len1 - dir1.Length())/2/dir1.Length();
            // grow dir1
            dir1 *= new_len1/dir1.Length();
        }
        else
        {
            // grow view along len2
            float new_len2 = (1/screen_aspect)*dir1.Length();
            // shift origin
            origin -= dir2*(new_len2 - dir2.Length())/2/dir2.Length();
            // grow dir2
            dir2 *= new_len2/dir2.Length();
        }

        // copy results back
        view.origin = ToRawArray(origin);
        view.dir1 = ToRawArray(dir1);
        view.dir2 = ToRawArray(dir2);
        return view;
    }

    /** Convert color format from 0xAABBGGRR to 0xAARRGGBB (swap red/blue). */
    static uint ABGR_to_ARGB(uint color)
    {
        var r = color & 0xFF;
        var g = (color >> 8) & 0xFF;
        var b = (color >> 16) & 0xFF;
        var a = (color >> 24) & 0xFF;
        return (a << 24) | (r << 16) | (g << 8) | b;
    }

    public static System.Drawing.Bitmap CreateBitmap(Image2d rgb_img)
    {
        Debug.Assert(rgb_img.format == ImageFormat.IMAGE_FORMAT_R8G8B8A8);

        // create target bitmap
        var bmp = new System.Drawing.Bitmap(rgb_img.dims[0], rgb_img.dims[1], System.Drawing.Imaging.PixelFormat.Format32bppArgb);
        {
            // lock bitmap to access data
            var data = bmp.LockBits(new Rectangle(0, 0, bmp.Width, bmp.Height), System.Drawing.Imaging.ImageLockMode.ReadWrite, bmp.PixelFormat);
            // create temp buffer with matching stride
            byte[] buffer = new byte[data.Height * data.Stride];
            for (int y = 0; y < data.Height; ++y)
            {
                for (int x = 0; x < data.Width; ++x)
                {
                    var img_offset = 4 * x + y * rgb_img.stride0;
                    var r = rgb_img.data[img_offset + 0];
                    var g = rgb_img.data[img_offset + 1];
                    var b = rgb_img.data[img_offset + 2];
                    var a = rgb_img.data[img_offset + 3];

                    var buffer_offset = 4 * x + y * data.Stride;
                    buffer[buffer_offset + 0] = b;
                    buffer[buffer_offset + 1] = g;
                    buffer[buffer_offset + 2] = r;
                    buffer[buffer_offset + 3] = a;
                }
            }
            // copy temporary buffer into bitmap
            System.Runtime.InteropServices.Marshal.Copy(buffer, 0, data.Scan0, buffer.Length);
            bmp.UnlockBits(data);
        }

        return bmp;
    }

    public static System.Drawing.Bitmap CreateBitmap(Image2d img, Image2d tissue_cm)
    {
        Debug.Assert(img.format == ImageFormat.IMAGE_FORMAT_U8);
        Debug.Assert(tissue_cm.format == ImageFormat.IMAGE_FORMAT_R8G8B8A8); // corresponds to 0xAABBGGRR

        // create target bitmap
        var bmp = new System.Drawing.Bitmap(img.dims[0], img.dims[1], System.Drawing.Imaging.PixelFormat.Format8bppIndexed);
        {
            // lock bitmap to access data
            var data = bmp.LockBits(new Rectangle(0, 0, bmp.Width, bmp.Height), System.Drawing.Imaging.ImageLockMode.ReadWrite, bmp.PixelFormat);
            // create temp buffer with matching stride
            byte[] buffer = new byte[data.Height * data.Stride];
            for (int y = 0; y < data.Height; ++y)
            {
                for (int x = 0; x < data.Width; ++x)
                {
                    var val = img.data[x + y * img.stride0];
                    buffer[x + y * data.Stride] = val;
                }
            }
            // copy temporary buffer into bitmap
            System.Runtime.InteropServices.Marshal.Copy(buffer, 0, data.Scan0, buffer.Length);
            bmp.UnlockBits(data);
        }

        {
            // configure color-map palette
            System.Drawing.Imaging.ColorPalette pal = bmp.Palette; // take a copy
            Debug.Assert(pal.Entries.Length == tissue_cm.dims[0]);

            for (int i = 0; i < tissue_cm.dims[0]; ++i)
            {
                var color = BitConverter.ToUInt32(tissue_cm.data, 4 * i);
                pal.Entries[i] = Color.FromArgb((int)ABGR_to_ARGB(color));
            }
            bmp.Palette = pal; // apply
        }

        return bmp;
    }
}

using System;
using System.Windows.Forms;
using Microsoft.Win32;
using System.Runtime.InteropServices;
using AppAPI;
using System.Collections.Generic;

namespace WinFormsApp
{
    /** The following registry entries are written/cleared automatically during build:
     * HKEY_CLASSES_ROOT\<namespace>.<classname>\..
     * HKEY_CLASSES_ROOT\CLSID\{guid}\...                   */
    [Guid("B8AE7209-83FE-4035-966D-21AC0DA1DCE4"), ComVisible(true)] // MUST be unique for each app
    public partial class PluginControl: UserControl, IPluginApp
    {
        private static readonly SRCode MEAS_METHOD = new SRCode{value="WinFormsApp", scheme="99TEST", meaning="Sample WinFormsApp"};
        private static readonly SRCode FINDING_SITE = new SRCode{value="T-32600", scheme="SRT", meaning="Left Ventricle"};


        private ComRefs refs = null;

        public PluginControl()
        {
            refs = new ComRefs();
            InitializeComponent();
        }

        protected override void OnHandleDestroyed(EventArgs e)
        {
            // release outstanding host references to allow GC to clean up
            refs = null;

            // release references to the host
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }

        public void Initialize(StartupMode startup_mode, InputMode input_mode, string locale, IPluginHostBasic basic_host, IPluginHostPatient pat_host)
        {
            refs.basic_host = basic_host;
            refs.pat_host = pat_host;

            // test logging
            IPluginLogger log = basic_host.GetLogReceiver();
            log.Log(MessageSeverity.MSG_INFO, "Hello world from WinFormsApp.UserControl1");

            // create touchpanel button & rotary
            try {
                IUserControls userControls = basic_host.GetUserControls();
                refs.test_button = (IControllerButton)userControls.CreateUserControl(UserControlType.USER_CONTROL_BUTTON, "Test button");
                refs.test_rotary = (IControllerSelectorFloat)userControls.CreateUserControl(UserControlType.USER_CONTROL_ROTARY, "Test rotary");
                refs.test_rotary.SetValues(new float[] { 1, 2, 3, 4, 5 }, 0);
            } catch (NotImplementedException) {
                // TouchPanel not accessible on EchoPAC.
            }

            LoadPreviewImage();
        }

        public void OnColorMapChanged(ColorMapType type, int MovieGroup, int movie)
        {
        }

        public void OnImageSourceChanged()
        {
        }

        public void OnImageSliceViewChanged()
        {
        }

        public void OnCustomViewSettingsChanged()
        {
        }

        public void OnUserControlChanged(IControllerBasic ctrl, UserEventType evt)
        {
            IPluginLogger log = refs.basic_host.GetLogReceiver();

            // use ComMarshal.EqualComObjects instead of == for comparison if objects have been created by a non-main thread
            if ((ctrl == refs.test_button) && (evt == UserEventType.USER_EVENT_CLICK)) {
                log.Log(MessageSeverity.MSG_INFO, "Button clicked");
            } else if ((ctrl == refs.test_rotary) && (evt == UserEventType.USER_EVENT_VALUE_CHANGE)) {
                log.Log(MessageSeverity.MSG_INFO, String.Format("Rotary changed to index {0}", refs.test_rotary.GetIndex()));
            }
        }

        public void OnDisplayModeChanged(DisplayMode display_mode)
        {
        }

        public void OnCurrentFrameChanged(int mg, int m, int s)
        {
        }

        struct RegEntry {
            public string path, value;

            public RegEntry(string _path, string _value) {
                path = _path; value = _value;
            }
        }

        static List<RegEntry> ExtraRegKeys(Type t)
        {
            List<RegEntry> keys = new List<RegEntry>();
            keys.Add(new RegEntry(@"CLSID\{" + t.GUID + @"}\Control", "")); // tag COM component as ActiveX control
            keys.Add(new RegEntry(@"CLSID\{" + t.GUID + @"}\Implemented Categories\{CDD621FE-F252-4BA1-BC87-15D19CAFCEAE}", "")); // preview image
            keys.Add(new RegEntry(@"CLSID\{" + t.GUID + @"}\Implemented Categories\{9173BC66-7961-4A2D-A732-095928D00ADC}", "")); // 2D image support
            keys.Add(new RegEntry(@"CLSID\{" + t.GUID + @"}\Version", string.Format("{0}.{1}", (int)AppAPIVersion.APPAPI_VERSION_MAJOR, (int)AppAPIVersion.APPAPI_VERSION_MINOR))); // AppAPI version
            return keys;
        }

        /** Entry-poing for adding ADDITIONAL registry entries (in addition to "InProcServer32" and "ThreadingModel"). */
        [ComRegisterFunction]
        public static void RegisterClass(Type t)
        {
            // Add additional registry keys
            foreach(RegEntry e in ExtraRegKeys(t)) {
                RegistryKey key = Registry.ClassesRoot.CreateSubKey(e.path);
                if (e.value.Length > 0)
                    key.SetValue(null, e.value);
                key.Close();
            }
#if false
            MessageBox.Show("Registering "+t+" (GUID: "+t.GUID+")"); // debugging aid
#endif
        }

        /** Entry-poing for removing ADDITIONAL registry entries. */
        [ComUnregisterFunction]
        public static void UnregisterClass(Type t)
        {
            foreach (RegEntry e in ExtraRegKeys(t)) {
                Registry.ClassesRoot.DeleteSubKey(e.path);
            }
#if false
            MessageBox.Show("UnRegistering "+t+" (GUID: "+t.GUID+")"); // debugging aid
#endif
        }

        private void LoadPreviewImage()
        {
            // test image acces
            IImageSource source = refs.pat_host.GetImageSource();
            Interval source_range = source.GetRange();
            if (source_range.end == source_range.begin)
                return; // no image data available

            // get 1st movie-group
            IMovieGroup mg = source.GetMovieGroup(source_range.begin);

            try {
                Image2d preview = mg.GetPreviewImage();
                pictureBox1.Image = ImageUtil.CreateBitmap(preview);
            } catch (Exception) {
                // ignore missing preview image
            }
        }

        private void buttonImage_Click(object sender, EventArgs e)
        {
            // test image acces
            IImageSource source = refs.pat_host.GetImageSource();
            Interval source_range = source.GetRange();
            if (source_range.end == source_range.begin)
                return; // no image data available

            // get 1st movie-group
            IMovieGroup mg = source.GetMovieGroup(source_range.begin);

            // iterate over all movies
            Interval m_range = mg.GetRange();
            for (int m_idx = m_range.begin; m_idx < m_range.end; ++m_idx)
            {
                ImageMovieType movie_type;
                object movie_dyn = mg.GetMovie(m_idx, ProcessingLevel.PROCESSING_CARTESIAN, out movie_type);
                if (movie_type != ImageMovieType.IMAGE_MOVIE_2D)
                    continue;
                IImage2dMovie movie = (IImage2dMovie)movie_dyn;

                Image2d tissue_cm = movie.GetColorMap(ColorMapType.TYPE_TISSUE_COLOR);

                uint[] max_res = { (uint)pictureBox1.Width, (uint)pictureBox1.Height };
                Cart2dGeomData bbox = movie.GetBoundingBox();
                bbox = ImageUtil.GrowViewToFitAspectRatio(bbox, max_res);

                Interval s_range = movie.GetRange();

                // iterate over all streams
                for (int s_idx = s_range.begin; s_idx < s_range.end; ++s_idx)
                {
                    IImage2dStream stream = movie.GetStream(m_range.begin, bbox, max_res);

                    ImageType img_type = stream.GetImageType();
                    if (img_type != ImageType.IMAGE_TYPE_TISSUE)
                        continue;

                    // get 1st frame
                    Interval f_range = stream.GetRange();
                    IImage2dFrame frame = stream.GetFrame(f_range.begin);
                    Image2d data = frame.GetData();
                    pictureBox1.Image = ImageUtil.CreateBitmap(data, tissue_cm);
                    break;
                }

                break; // only display first matching movie
            }
        }

        private void buttonImageRendered_Click(object sender, EventArgs e)
        {
            // test image acces
            IImageSource source = refs.pat_host.GetImageSource();
            Interval source_range = source.GetRange();
            if (source_range.end == source_range.begin)
                return; // no image data available

            // get 1st movie-group
            IMovieGroup mg = source.GetMovieGroup(source_range.begin);

            ImageMovieType movie_type;
            const int first_movie_idx = 0;
            object movie_dyn = mg.GetMovie(first_movie_idx, ProcessingLevel.PROCESSING_RENDERED, out movie_type);
            IImageRenderedMovie movie = (IImageRenderedMovie)movie_dyn;

            uint[] max_res = { (uint)pictureBox1.Width, (uint)pictureBox1.Height };
            Cart2dGeomData bbox = movie.GetBoundingBox();
            bbox = ImageUtil.GrowViewToFitAspectRatio(bbox, max_res);

            IImage2dStream stream;
            try {
                stream = movie.GetStream(bbox, max_res);
            }
            catch (COMException) {
                // When custom resolution is not supported (e.g. pure DICOM) fallback to the default stream
                stream = movie.GetDefaultStream();
            }

            // get 1st frame
            Interval f_range = stream.GetRange();
            IImage2dFrame frame = stream.GetFrame(f_range.begin);
            Image2d data = frame.GetData();
            pictureBox1.Image = ImageUtil.CreateBitmap(data);
        }

        private void buttonGenMeas_Click(object sender, EventArgs e)
        {
            // test measurements
            {
                IMeasurementSeries series = refs.pat_host.CreateMeasSeries(MEAS_METHOD);
                IMeasurementFindings findings = series.GetFindings(FINDING_SITE);

                var data = default(MeasurementData);
                data.disp = "EDV";
                data.type = new SRCode{value="18026-5", scheme="LN", meaning="Left Ventricular End Diastolic Volume"};
                data.num_val = 100f;
                data.unit = UnitType.UNIT_VOLUME_ML;
                data.image_view = new SRCode{value="G-A19C", scheme="SRT", meaning="Apical Four Chamber"};
                findings.Add(data);
            }

            // test patient data access
            IPatientData pat = refs.pat_host.GetPatientDataSource();
            try
            {
                string name = pat.GetParamStr(PatientParamStr.PAT_NAME_FIRST);
            }
            catch (Exception)
            {
                // patient name not available
            }
        }

        private void buttonLogMeas_Click(object sender, EventArgs e)
        {
            // perform calls from a separate thread to demonstrate multithreaded AppAPI access
            _ = ComTask.Run<object>(System.Threading.ApartmentState.MTA, () => {

                IPluginLogger log = refs.basic_host.GetLogReceiver();

                IMeasurementSeries series = refs.pat_host.CreateMeasSeries(MEAS_METHOD);
                IMeasurementFindings findings = series.GetFindings(FINDING_SITE);

                uint meas_count = findings.GetSize();
                log.Log(MessageSeverity.MSG_INFO, String.Format("Measurement count: {0}", meas_count));
                for (uint i = 0; i < meas_count; i++) {
                    MeasurementData data = findings.Get(i);
                    log.Log(MessageSeverity.MSG_INFO, String.Format("Measurement: display={0}, value={1}, unit={2}", data.disp, data.num_val, data.unit));
                }

                return null;
            });
        }
    }

    /** Host references that need to be released before calling the GC in OnHandleDestroyed */
    class ComRefs
    {
        public IPluginHostBasic basic_host = null;
        public IPluginHostPatient pat_host = null;
        public IControllerButton test_button = null;
        public IControllerSelectorFloat test_rotary = null;
    }
}

#pragma once
#ifndef Q_MOC_RUN
  #include <AppAPI/AppAPI.tlh>
#endif
#ifdef __ANDROID__
  #include <QTemporaryFile>
  #include <QFileInfo>
#endif

inline void LoadFileIntoSource (std::wstring filename, AppAPI::IImageSource & source) {
    AppAPI::IFileLoaderPtr loader;
    loader = AppAPI::IImageSourcePtr(&source);
    
    CComPtr<AppAPI::IMovieGroup> mg; // CComPtr for CComSafeArray compatibility
    {
        SAFEARRAY * sa_tmp = loader->OpenFile(_bstr_t(filename.c_str()));
        CComSafeArray<IUnknown*> mgs;
        mgs.Attach(sa_tmp);
        sa_tmp = nullptr;
        mg = mgs.GetAt(0); // select 1st movie-group from loader
    }
    int out_idx = 0;
    CHECK(loader->AddMovieGroup(mg, &out_idx));
}

inline void GenerateSyntheticIntoSource (AppAPI::IImageSource & source) {
    AppAPI::IFileLoaderPtr loader;
    loader = AppAPI::IImageSourcePtr(&source);
    
    // use simulator on mobile platforms until we get suitable file selection UI
    AppAPI::IMovieGroupPtr mg = loader->GenerateSyntheticData(2/*2D*/, AppAPI::IMAGE_TYPE_TISSUE, 0/*seed*/);
    
    int out_idx = 0;
    CHECK(loader->AddMovieGroup(mg, &out_idx));
}

#ifdef __ANDROID__
/* This function is a macro to overcome the limitation of QTemporaryFile not being movable.
   A regular function therefore cannot return a QTemporaryFile. */
#define CopyContentURIToTempFile(filepath) \
    QTemporaryFile tmpFile; /* file auto-deleted in dtor.*/ \
    QFileInfo file_info(filepath); \
    if (!file_info.suffix().isEmpty()) \
        tmpFile.setFileTemplate("XXXXXX." + file_info.suffix()); /* Preserve extension.*/ \
    if (tmpFile.open()) { \
        /* copy content URI file to NDK-accessible temp. file */ \
        QFile srcFile(filepath); /* QFile includes special "content://" support since https://codereview.qt-project.org/#/c/251038/ */ \
        if (!srcFile.open(QIODevice::ReadOnly)) \
            abort(); \
        tmpFile.write(srcFile.readAll()); \
        tmpFile.close(); \
        /* update filename to point to the temp file */ \
        filepath = tmpFile.fileName(); \
    }
#endif

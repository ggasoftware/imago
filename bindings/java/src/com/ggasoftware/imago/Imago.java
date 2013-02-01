/****************************************************************************
 * Copyright (C) 2011 GGA Software Services LLC
 * 
 * This file is part of Imago toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

package com.ggasoftware.imago;

import com.sun.jna.*;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.awt.image.DataBuffer;
import java.io.*;
import java.lang.reflect.*;
import java.util.*;

public class Imago {

    public enum Filters {
        STD("prefilter_basic"),  // could be updated according filters_list.cpp
        ADAPTIVE("prefilter_adaptive"),
        CV("prefilter_basic"),
        PASSTHRU("prefilter_binarized");
        
        private String filterName;
        Filters(String name) {
            this.filterName = name;
        }
        
        public String getName() {
            return this.filterName;
        }
    }

    public class LogRecord {
        public String filename = null;
        public byte[] data = null;

        public LogRecord(String filename, byte[] data) {
            this.filename = filename;
            this.data = data;
        }

        public LogRecord() {
        }
    }
    
    static private int checkResult(int result) {
        if (result < 0) {
            throw new ImagoException(_lib.imagoGetLastError());
        }

        return result;
    }

    public String getVersion() {
        return _lib.imagoGetVersion();
    }
    
    public void setSessionID()
    {
        _lib.imagoSetSessionId(_sid);
    }

    public void enableLog(boolean virtual) {
        setSessionID();

        if (virtual)
            _lib.imagoSetLogging(2);
        else
            _lib.imagoSetLogging(1);
    }
    
    public void disableLog() {
        setSessionID();
        _lib.imagoSetLogging(0);
    }

    public void setConfig(String config) {
        setSessionID();
        checkResult(_lib.imagoSetConfig(config));
    }

    public String getConfigsList() {
        setSessionID();
        return _lib.imagoGetConfigsList();
    }
    
    public void setFilter(Filters filter) {
        setSessionID();
        checkResult(_lib.imagoSetFilter(filter.getName()));
    }
    
    public void filterImage() {
        setSessionID();
        checkResult(_lib.imagoFilterImage());
    }
    
    public Image getFilteredImage() {
        IntByReference width = new IntByReference(),
                       height = new IntByReference();        
        PointerByReference data = new PointerByReference(Pointer.NULL);
        _lib.imagoGetPrefilteredImage(data, width, height);
        
        int w = width.getValue(), h = height.getValue();
        BufferedImage img = new BufferedImage(w, h, BufferedImage.TYPE_BYTE_GRAY);
        for (int i = 0; i < width.getValue(); i++) {
            for (int j = 0; j < height.getValue(); j++) {
                img.setRGB(i, j, data.getValue().getByte(j * w + i));
            }
        }
        return img;
    }
    
    public void recognize() {
        setSessionID();
        IntByReference warnings = new IntByReference();
        checkResult(_lib.imagoRecognize(warnings));
    }
    
    public String getResultMolecule() {
        setSessionID();

        PointerByReference result = new PointerByReference(Pointer.NULL);
        IntByReference size = new IntByReference(0);

        checkResult(_lib.imagoSaveMolToBuffer(result, size));

        return result.getValue().getString(0);
    }
    
    public void saveImage(String filename) {
        setSessionID();
        checkResult(_lib.imagoSaveImageToFile(filename));
    }

    public void loadImage(String filename) {
        setSessionID();
        checkResult(_lib.imagoLoadImageFromFile(filename));
    }

    public void loadImage(byte[] buffer) {
        setSessionID();
        checkResult(_lib.imagoLoadImageFromBuffer(buffer, buffer.length));
    }

    public void loadImage(BufferedImage image) throws ImagoException {
        setSessionID();
        BufferedImage img = new BufferedImage(image.getWidth(), image.getHeight(),
            BufferedImage.TYPE_BYTE_GRAY);  
        Graphics g = img.getGraphics();  
        g.drawImage(image, 0, 0, null);  
        g.dispose(); 

        DataBuffer buf = img.getData().getDataBuffer();
        byte[] simple_buf = new byte[buf.getSize()];  
        for (int j = 0; j < buf.getSize(); j++) {
            simple_buf[j] = (byte)buf.getElem(j);
        }
            
        int w = img.getWidth(), h = img.getHeight();

        checkResult(_lib.imagoLoadGreyscaleRawImage(simple_buf, w, h));
    }

    public LogRecord[] getLogRecords() {
        setSessionID();

        IntByReference count = new IntByReference();
        _lib.imagoGetLogCount(count);
        
        if (count.getValue() == 0)
            return null;

        PointerByReference name = new PointerByReference(),
                data = new PointerByReference();
        IntByReference length = new IntByReference();
        LogRecord[] log = new LogRecord[count.getValue()];

        for (int i = 0; i < count.getValue(); ++i) {
            _lib.imagoGetLogRecord(i, name, length, data);
            log[i] = new LogRecord(name.getValue().getString(0),
                    data.getValue().getByteArray(0, length.getValue()));
        }

        return log;
    }
    
    public static class LibraryRemover {

        ArrayList<String> files = new ArrayList<String>();
        ArrayList<String> directories = new ArrayList<String>();

        public LibraryRemover() {
            final LibraryRemover self = this;

            Runtime.getRuntime().addShutdownHook(new Thread() {

                @Override
                public void run() {
                    self.removeLibraries();
                }
            });
        }

        public synchronized void addLibrary(String directory, String fullpath) {
            files.add(fullpath);
            directories.add(directory);

            if (_os == OS_WINDOWS) {
                // The caller can load our DLL file with System.load() OR with
                // Native.loadLibrary(). To get the mess below working in the second
                // case, we call System.load() by ourselves. This makes the library
                // listed in the hidden ClassLoader.nativeLibraries field.
                System.load(fullpath);
            }
        }

        public synchronized void removeLibraries() {
            for (int idx = files.size() - 1; idx >= 0; idx--) {
                String fullpath = files.get(idx);

                if (_os == OS_WINDOWS) {
                    // In Windows, we can not remove the DLL file until we unload
                    // it from the process. Nobody cares that the DLL files are
                    // usually read into memory and the process does not need them
                    // on the disk.
                    try {
                        ClassLoader cl = Imago.class.getClassLoader();
                        Field f = ClassLoader.class.getDeclaredField("nativeLibraries");
                        f.setAccessible(true);
                        List libs = (List) f.get(cl);
                        for (Iterator i = libs.iterator(); i.hasNext();) {
                            Object lib = i.next();
                            f = lib.getClass().getDeclaredField("name");
                            f.setAccessible(true);
                            String name = (String) f.get(lib);
                            if (name.equals(fullpath)) {
                                Method m = lib.getClass().getDeclaredMethod("finalize", new Class[0]);
                                m.setAccessible(true);
                                // Here comes the trick: we call the finalizer twice,
                                // first time to undo our own System.load() above, and
                                // the second time to undo
                                // Native.loadLibrary/System.load() done by the caller.
                                // Each finalize() call decrements the "reference
                                // counter" of the process for the DLL file. After the
                                // counter is zero, the deletion of the file becomes
                                // possible.
                                m.invoke(lib, new Object[0]);
                                m.invoke(lib, new Object[0]);
                            }
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                (new File(fullpath)).delete();
                (new File(directories.get(idx))).delete();
            }
        }
    }
    static LibraryRemover _library_remover = new Imago.LibraryRemover();

    public static String extractFromJar(Class cls, String path, String filename) {
        InputStream stream = cls.getResourceAsStream(path + "/" + filename);

        if (stream == null) {
            return null;
        }

        String tmpdir_path;

        try {
            File tmpfile = File.createTempFile("imago", null);
            tmpdir_path = tmpfile.getAbsolutePath() + ".d";
            tmpfile.delete();
        } catch (IOException e) {
            return null;
        }

        final File tmpdir = new File(tmpdir_path);
        if (!tmpdir.mkdir()) {
            return null;
        }

        final File dllfile = new File(tmpdir.getAbsolutePath() + File.separator + filename);

        try {
            FileOutputStream outstream = new FileOutputStream(dllfile);
            byte buf[] = new byte[4096];
            int len;

            while ((len = stream.read(buf)) > 0) {
                outstream.write(buf, 0, len);
            }

            outstream.close();
            stream.close();
        } catch (IOException e) {
            return null;
        }

        String p;

        try {
            p = dllfile.getCanonicalPath();
        } catch (IOException e) {
            return null;
        }

        final String fullpath = p;

        // To remove the temporary file and the directory on program's exit.
        _library_remover.addLibrary(tmpdir_path, fullpath);

        return fullpath;
    }

    private static String getPathToBinary(String path, String filename) {
        if (path == null) {
            String res = extractFromJar(Imago.class, "/com/ggasoftware/imago/" + _dllpath, filename);
            if (res != null) {
                return res;
            }
            path = "lib";
        }
        path = path + File.separator + _dllpath + File.separator + filename;
        try {
            return (new File(path)).getCanonicalPath();
        } catch (IOException e) {
            return path;
        }
    }

    private synchronized static void loadImago(String path) {
        if (_lib != null) {
            return;
        }

        if (_os == OS_LINUX || _os == OS_SOLARIS) {
            _lib = (ImagoLib) Native.loadLibrary(getPathToBinary(path, "libimago_c.so"), ImagoLib.class);
        } else if (_os == OS_MACOS) {
            _lib = (ImagoLib) Native.loadLibrary(getPathToBinary(path, "libimago_c.dylib"), ImagoLib.class);
        } else // _os == OS_WINDOWS
        {
            System.load(getPathToBinary(path, "msvcr100.dll"));
            _lib = (ImagoLib) Native.loadLibrary(getPathToBinary(path, "imago_c.dll"), ImagoLib.class);
        }
    }

    public Imago(String path)
    {
        _path = path;
        loadImago(path);

        _sid = _lib.imagoAllocSessionId();
    }

    public Imago()
    {
        this(null);
    }

    public String getUserSpecifiedPath ()
    {
        return _path;
    }

    static public String getPlatformDependentPath ()
    {
        return _dllpath;
    }

    public long getSid ()
    {
        return _sid;
    }

    @Override
    @SuppressWarnings("FinalizeDeclaration")
    protected void finalize () throws Throwable
    {
        _lib.imagoReleaseSessionId(_sid);
        super.finalize();
    }

    private String _path;
    private long _sid;
    
    private static final int OS_WINDOWS = 1;
    private static final int OS_MACOS = 2;
    private static final int OS_LINUX = 3;
    private static final int OS_SOLARIS = 4;
    private static int _os = 0;
    private static String _dllpath = "";
    private static ImagoLib _lib = null;

    public static ImagoLib getLibrary() {
        return _lib;
    }

    private static int getOs() {
        String namestr = System.getProperty("os.name");
        if (namestr.matches("^Windows.*")) {
            return OS_WINDOWS;
        } else if (namestr.matches("^Mac OS.*")) {
            return OS_MACOS;
        } else if (namestr.matches("^Linux.*")) {
            return OS_LINUX;
        } else {
            throw new Error("Operating system not recognized");
        }
    }

    private static String getDllPath() {
        String path = "";
        switch (_os) {
            case OS_WINDOWS:
                path += "Win";
                break;
            case OS_LINUX:
                path += "Linux";
                break;
            case OS_MACOS:
                path += "Mac";
                break;
            default:
                throw new Error("OS not set");
        }
        path += "/";

        if (_os == OS_MACOS) {
            String version = System.getProperty("os.version");

            if (version.startsWith("10.5")) {
                path += "10.5";
            } else if (version.startsWith("10.6")) {
                path += "10.6";
            } else {
                throw new Error("OS version not supported");
            }
        } else {
            String archstr = System.getProperty("os.arch");
            if (archstr.equals("x86") || archstr.equals("i386")) {
                path += "x86";
            } else if (archstr.equals("x86_64") || archstr.equals("amd64")) {
                path += "x64";
            } else {
                throw new Error("architecture not recognized");
            }
        }

        return path;
    }

    static {
        _os = getOs();
        _dllpath = getDllPath();
    }
}

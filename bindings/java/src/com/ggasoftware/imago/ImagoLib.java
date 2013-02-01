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
import com.sun.jna.ptr.*;

public interface ImagoLib extends Library {
    String imagoGetVersion();
    String imagoGetLastError();

    long imagoAllocSessionId();
    void imagoSetSessionId(long id);
    void imagoReleaseSessionId(long id);
    
    int imagoSetConfig(String config);
    String imagoGetConfigsList();

    int imagoSetFilter(String filter);

    int imagoLoadImageFromBuffer(byte[] buf, int buf_size);
    int imagoLoadImageFromFile(String filename);

    int imagoSaveImageToFile(String filename);

    int imagoLoadGreyscaleRawImage(byte[] buf, int width, int height);

    int imagoSetLogging(int mode);

    int imagoRecognize(IntByReference warnings);

    int imagoSaveMolToBuffer(PointerByReference buf, IntByReference buf_size);
    int imagoSaveMolToFile(String filename);

    int imagoFilterImage();

    int imagoGetInkPercentage(DoubleByReference percentage);

    int imagoGetPrefilteredImageSize(IntByReference width, IntByReference height);
    int imagoGetPrefilteredImage(PointerByReference data, IntByReference width, IntByReference height);

    int imagoSetSessionSpecificData(Pointer data);
    int imagoGetSessionSpecificData(PointerByReference data);
    
    int imagoGetLogCount(IntByReference count);
    int imagoGetLogRecord(int it, PointerByReference name, IntByReference length, PointerByReference data);
}
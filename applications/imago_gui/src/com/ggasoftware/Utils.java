/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
 *
 * This file is part of Imago OCR project.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

 package com.ggasoftware;

import java.io.File;

public class Utils {
    public static String getFileExtension(File file) {
        String name = file.getName();
        if (!name.contains(".")) {
            return null;
        }
        return name.substring(name.lastIndexOf('.') + 1).toLowerCase();
    }

    public static boolean checkFile(File file) {
        if (isPDF(file)) {
            return true;
        }
        if (isTIF(file)) {
            return true;
        }
        if (isAcceptableImage(file)) {
            return true;
        }
        return false;
    }

    public static boolean isPDF(File file) {
        if ("pdf".equals(getFileExtension(file))) {
            return true;
        }
        return false;
    }

    public static boolean isTIF(File file) {
        String ext = getFileExtension(file);
        if ("tif".equals(ext) || "tiff".equals(ext)) {
            return true;
        }
        return false;
    }

    public static boolean isAcceptableImage(File file) {
        String ext = getFileExtension(file);
        if ("png".equals(ext) || "jpg".equals(ext) || "gif".equals(ext)) {
            return true;
        }
        return false;
    }
    
}

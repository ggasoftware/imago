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

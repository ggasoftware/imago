/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

package com.ggasoftware.DocumentHandling;

import com.sun.media.jai.codec.FileSeekableStream;
import com.sun.media.jai.codec.ImageCodec;
import com.sun.media.jai.codec.ImageDecoder;
import com.sun.media.jai.codec.TIFFDecodeParam;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.awt.image.ImageObserver;
import java.io.File;
import java.io.IOException;
import javax.media.jai.NullOpImage;
import javax.media.jai.OpImage;

public class TiffDocument implements Document {

    public class TiffDocumentAsImage implements DocumentPageAsImage {

        NullOpImage noImage;
        BufferedImage origImage;
        BufferedImage image;
        double scale;
        
        public TiffDocumentAsImage(NullOpImage noImg) {
            noImage = noImg;
            origImage = noImage.getAsBufferedImage();
            image = origImage;
        }

        public void paint(Graphics g) {
            if (image != null) {
                g.drawImage(image, 0, 0, null);
                g.setColor(Color.black);
                g.drawRect(-1, -1, image.getWidth(), image.getHeight());
            }
        }

        public Dimension getSize() {
            return new Dimension(image.getWidth(), image.getHeight());
        }

        public Dimension getUnscaledSize() {
            return new Dimension(noImage.getWidth(), noImage.getHeight());
        }

        public void setScale(double scl) {
            scale = scl;
            int newWidth = (int)(noImage.getWidth() * scale);
            int newHeight = (int)(noImage.getHeight() * scale);
            int type = (origImage.getType() == 0) ? BufferedImage.TYPE_INT_RGB : origImage.getType();

            image = new BufferedImage(newWidth, newHeight, type);
            image.getGraphics().drawImage(origImage.getScaledInstance(newWidth,
                    newHeight, Image.SCALE_FAST), 0, 0, null);
        }

        public BufferedImage getSelectedRectangle(Rectangle rect,
                ImageObserver observer) {
            return image.getSubimage(rect.x, rect.y, rect.width, rect.height);
        }
    }

    private ImageDecoder dec;
    private FileSeekableStream stream;
    public TiffDocument(File file) throws IOException {
        stream = new FileSeekableStream(file);
        TIFFDecodeParam param = null;
        dec = ImageCodec.createImageDecoder("tiff", stream, param);
    }

    public int getPageCount() {
        try {
            return dec.getNumPages();
        } catch (IOException e) {
            System.out.println("Caught an exeption in TIFF.getPageCount()\n" +
                    e.getMessage());
            return -1;
        }
    }

    public DocumentPageAsImage getPage(int page, ImageObserver observer) {
        try {
            NullOpImage op =
                new NullOpImage(dec.decodeAsRenderedImage(page - 1),
                                null,
                                OpImage.OP_IO_BOUND,
                                null);
            return new TiffDocumentAsImage(op);
        } catch(IOException e) {
            System.out.println("Caught an exeption in TIFF.getScaledPage()\n" +
                    e.getMessage());
            return null;
        }
    }

}

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

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.awt.image.ImageObserver;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;

public class ImageDocument implements Document {

    private class ImagePage implements DocumentPageAsImage {

        private BufferedImage origImage;
        private BufferedImage image;
        private double scl;

        public ImagePage(BufferedImage img) {
            
            //TODO: Not sure if this right in all cases.
            if (img.getAlphaRaster() != null) {
                BufferedImage tmp = new BufferedImage(img.getWidth(), 
                        img.getHeight(), BufferedImage.TYPE_3BYTE_BGR);
                
                tmp.getGraphics().drawImage(img, 0, 0, Color.white, null);
                img = tmp;
            }
            
            scl = -1;

            origImage = img;
            image = origImage;
        }

        public Dimension getSize() {
            return new Dimension(image.getWidth(), image.getHeight());
        }

        public Dimension getUnscaledSize() {
            return new Dimension(origImage.getWidth(), origImage.getHeight());
        }

        public void paint(Graphics g) {
            if (image != null) {
                g.drawImage(image, 0, 0, null);
                g.setColor(Color.black);
                g.drawRect(-1, -1, image.getWidth() + 1, image.getHeight() + 1);
            }
        }

        public void setScale(double scale) {
            if (Math.abs(scl - scale) > 0.001 || scl == -1)
            {
                int newWidth = (int)(origImage.getWidth() * scale);
                int newHeight = (int)(origImage.getHeight() * scale);


                Image scaled = origImage.getScaledInstance(newWidth, newHeight,
                        Image.SCALE_FAST);
                image = new BufferedImage(scaled.getWidth(null), scaled.getHeight(null),
                        origImage.getType());
                image.getGraphics().drawImage(scaled, 0, 0, null);
                
                scl = scale;
            }
        }

        public BufferedImage getSelectedRectangle(Rectangle rect, ImageObserver observer) {
            return image.getSubimage(rect.x, rect.y, rect.width, rect.height);
        }

    }

    ImagePage ipage;

    public ImageDocument(File file) {
        try {
            ipage = new ImagePage(ImageIO.read(file));
        } catch (IOException e) {
        }
    }

    public ImageDocument(BufferedImage img) {
        ipage = new ImagePage(img);
    }

    public int getPageCount() {
        return 1;
    }

    public DocumentPageAsImage getPage(int page, ImageObserver observer) {
        return ipage;
    }


}

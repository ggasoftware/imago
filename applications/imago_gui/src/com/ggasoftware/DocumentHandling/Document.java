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

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.awt.image.ImageObserver;

public interface Document {

    public interface DocumentPageAsImage {
        public void paint(Graphics g);
        public BufferedImage getSelectedRectangle(Rectangle rect,
                ImageObserver observer);

        public void setScale(double scale);
        public Dimension getSize();
        public Dimension getUnscaledSize();
    }

    public int getPageCount();
    public DocumentPageAsImage getPage(int page, ImageObserver observer);
}

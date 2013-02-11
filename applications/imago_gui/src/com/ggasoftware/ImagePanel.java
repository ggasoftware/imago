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

package com.ggasoftware;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Image;
import javax.swing.JPanel;

public class ImagePanel extends javax.swing.JPanel {

    private Image orig_img = null;
    private Image scaled_img = null;

    void ImagePanel() {
    }

    private void scaleImage() {
        if (orig_img == null)
            return;

        scaled_img = null;
        
        int i_height = orig_img.getHeight(this);
        int i_width = orig_img.getWidth(this);

        if (i_height > i_width) {
            if (i_height > getHeight()) {
                scaled_img = orig_img.getScaledInstance(-1, getHeight(), Image.SCALE_FAST);
            }
        }
        else {
            if (i_width > getWidth()) {
                scaled_img = orig_img.getScaledInstance(getWidth(), -1, Image.SCALE_FAST);
            }
        }
        if (scaled_img == null)
            scaled_img = orig_img;
    }

    void setImage(Image new_img) {
        orig_img = new_img;
        scaleImage();
    }
    
    @Override
    public void paint(Graphics g) {
        super.paint(g);

        if (scaled_img != null) {
            g.drawImage(scaled_img, (getWidth() - scaled_img.getWidth(this)) / 2, (getHeight() - scaled_img.getHeight(this)) / 2, this);
        }
        else {
            g.setColor(Color.WHITE);
            g.fillRect(0, 0, getWidth(), getHeight());
        }
    }
}

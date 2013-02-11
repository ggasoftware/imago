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

import java.awt.Component;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.*;
import java.awt.image.ImageObserver;
import com.ggasoftware.DocumentHandling.Document.DocumentPageAsImage;
import com.ggasoftware.EgoFrame;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.image.BufferedImage;
import javax.swing.*;

public class DocumentPanel extends javax.swing.JPanel {
    static private class ViewerPanel extends javax.swing.JPanel
            implements MouseListener, MouseMotionListener, MouseWheelListener, ImageObserver {

        private Point begin;
        private Rectangle selection = new Rectangle(), VisRect = new Rectangle();
        private DocumentPageAsImage currentPage;
        private double scale = -1, saved_scale = -1;
        private boolean ready = false, init = false, force_scroll = false;
        private DocumentPanel parent;

        public double[] available_scales = new double[12];

        public ViewerPanel(DocumentPanel prnt) {

            available_scales[0] = 0.125;
            available_scales[1] = 0.25;
            available_scales[2] = 0.333;
            available_scales[3] = 0.5;
            available_scales[4] = 0.666;
            available_scales[5] = 0.75;
            available_scales[6] = 1.0;
            available_scales[7] = 1.25;
            available_scales[8] = 1.5;
            available_scales[9] = 2.0;
            available_scales[10] = 3.0;
            available_scales[11] = 4.0;

            parent = prnt;

            selection.width = -1;
            selection.height = -1;

        }

        public void initListeners() {
            if (!isInit()) {
                addMouseListener(this);
                addMouseWheelListener(this);
                addMouseMotionListener(this);
                init = true;
            }
        }

        public boolean isInit() {
            return init;
        }

        public void viewPage(DocumentPageAsImage page) {
            selection.width = selection.height = 0;
            ready = false;
            currentPage = page;

            if (page.getUnscaledSize().height > page.getUnscaledSize().width)
                parent.fitHeight();
            else
                parent.fitWidth();

            ready = true;
        }

        @Override
        public void paint(Graphics g) {
            super.paint(g);

            if (ready)
            {
                Point center = new Point((parent.jPanel1.getVisibleRect().width -
                        currentPage.getSize().width) / 2,

                        (parent.jPanel1.getVisibleRect().height -
                                currentPage.getSize().height) / 2);

                if (center.x < 0)
                    center.x = 0;
                if (center.y < 0)
                    center.y = 0;

                g.translate(center.x, center.y);
                currentPage.paint(g);
                g.setColor(Color.blue);
                g.translate(-center.x, -center.y);
                g.drawRect(selection.x, selection.y, selection.width, selection.height);

                if (force_scroll)
                {
                    parent.jPanel1.scrollRectToVisible(VisRect);
                    force_scroll = false;
                }
            }
        }

        //TODO: Not really needed now
        @Override
        public boolean imageUpdate(Image img, int infoflags, int x, int y,
                int width, int height) {
            if ((infoflags & (SOMEBITS | ALLBITS)) != 0) {
                repaint(x, y, width, height);
            }

            if ((infoflags & (ALLBITS | ERROR | ABORT)) != 0) {
                return false;
            } else {
                return true;
            }
        }

        public void newDocument() {
            scale = 1.0;            
        }

        public void prepPage(double new_scale) {
            currentPage.setScale(new_scale);
            setPreferredSize(currentPage.getSize());

            int dx = (currentPage.getSize().width - parent.jScrollPane1.getSize().width) / 2,
                dy = (currentPage.getSize().height - parent.jScrollPane1.getSize().height) / 2;

            if (dx < 0)
                dx = 0;

            if (dy < 0)
                dy = 0;
            
            VisRect.x = dx;
            VisRect.y = dy;

            VisRect.width = (int)(parent.jScrollPane1.getSize().width);
            VisRect.height = (int)(parent.jScrollPane1.getSize().height);

            force_scroll = true;

            scale = new_scale;

            revalidate();
            repaint();
        }

        public double getScale() {
            return scale;
        }

        public int findClosestScale(double d, boolean positive_dir) {
            int index = -1;

            if (positive_dir) {

                double max = Double.MAX_VALUE;

                for (int i = 0; i < available_scales.length; i++) {
                    if (available_scales[i] - d > 0) {
                        if (available_scales[i] - d < max) {
                            max = available_scales[i] - d;
                            index = i;
                        }
                    }
                }
            }
            else {
                
                double min = -Double.MAX_VALUE;

                for (int i = 0; i < available_scales.length; i++) {
                    if (available_scales[i] - d < 0) {
                        if (available_scales[i] - d > min) {
                            min = available_scales[i] - d;
                            index = i;
                        }
                    }
                }
            }

            return index;
        }

        public BufferedImage getSelectedSubimage(ImageObserver observer) {
            if (selection.width == 0 || selection.height == 0)
            {
                return currentPage.
                        getSelectedRectangle(new Rectangle(currentPage.getSize().width,
                        currentPage.getSize().height), observer);
            }

            Point center =
                    new Point((parent.jPanel1.getVisibleRect().width -
                            currentPage.getSize().width) / 2,
                            (parent.jPanel1.getVisibleRect().height -
                                    currentPage.getSize().height) / 2);

            if (center.x > 0)
                selection.x -= center.x;
            
            if (center.y > 0)
                selection.y -= center.y;

            Rectangle s = new Rectangle(selection.x, selection.y,
                    selection.width, selection.height);
            s.height++;
            s.width++;
            saved_scale = scale;

            if (saved_scale < 1.0) {
                scale = 1.0;
                currentPage.setScale(scale);
                s.x = (int)((double)s.x / saved_scale);
                s.y = (int)((double)s.y / saved_scale);

                s.width = (int)((double)s.width / saved_scale);
                s.height = (int)((double)s.height / saved_scale);
                parent.jZoomComboBox.setSelectedItem("100%");
                parent.jZoomComboBox.setSelectedIndex(10);
            }
            else {
                scale = saved_scale;
            }

            return currentPage.getSelectedRectangle(s, observer);
        }

        //TODO: Strange functions.
        public void increaseScale() {
            int ind = parent.jZoomComboBox.getSelectedIndex(),
                count = parent.jZoomComboBox.getItemCount();            

            if (ind >= count - 1)
                return;

            if (ind - 4 < 0)
            {
                int i = findClosestScale(scale, true);

                if (i < 0)
                    return;

                parent.jZoomComboBox.setSelectedIndex(i + 4);
                prepPage(available_scales[i]);
                return;
            }

            ind++;

            parent.jZoomComboBox.setSelectedIndex(ind);
            parent.jZoomComboBox.getItemAt(ind);

            String cur_item = parent.jZoomComboBox.getItemAt(ind).toString();
            
            cur_item = cur_item.substring(0, cur_item.length() - 1);
            prepPage(Double.parseDouble(cur_item) / 100);
        }

        public void setActualSize() {
            prepPage(1.0);
            parent.jZoomComboBox.setSelectedItem("100%");
            parent.jZoomComboBox.setSelectedIndex(10);
        }

        public void decreaseScale() {
            int ind = parent.jZoomComboBox.getSelectedIndex();

            if (ind <= 4 && ind >= 0)
                return;

            if (ind < 0)
            {
                int i = findClosestScale(scale, false);

                if (i < 0)
                    return;
                
                parent.jZoomComboBox.setSelectedIndex(i + 4);
                prepPage(available_scales[i]);
                return;
            }

            ind--;

            parent.jZoomComboBox.setSelectedIndex(ind);
            parent.jZoomComboBox.getItemAt(ind);

            String cur_item = parent.jZoomComboBox.getItemAt(ind).toString();

            cur_item = cur_item.substring(0, cur_item.length() - 1);
            prepPage(Double.parseDouble(cur_item) / 100);
        }

        public void fitWidth() {
            double new_scale;
            Dimension d = currentPage.getUnscaledSize();

            if (parent.jScrollPane1.getVerticalScrollBar().isVisible()) {
                new_scale = (double)(parent.jPanel1.getVisibleRect().width)
                    / d.width;
            }
            else {
                new_scale = (double)(parent.jPanel1.getVisibleRect().width -
                parent.jScrollPane1.getVerticalScrollBar().getMaximumSize().width)
                    / d.width;
            }

            prepPage(new_scale);
        }

        public void fitHeight() {
            double new_scale;
            Dimension d = currentPage.getUnscaledSize();

            if (parent.jScrollPane1.getHorizontalScrollBar().isVisible()) {
                new_scale = (double)(parent.jPanel1.getVisibleRect().height)
                    / d.height;
            }
            else {
                new_scale = (double)(parent.jPanel1.getVisibleRect().height -
                    parent.jScrollPane1.getHorizontalScrollBar().getSize().height)
                    / d.height;
            }
            prepPage(new_scale);
        }

        public void recoverPreviousDocument() {
            currentPage = parent.document.getPage(parent.curPage, this);
            prepPage(saved_scale);
            viewPage(currentPage);
            parent.parent.toggleAfterSelectionItems(false);

            parent.jZoomComboBox.setSelectedItem(
                String.valueOf((float)(getScale() * 100)) + "%");
        }

        public void mouseClicked(MouseEvent me) {
        }

        public void mousePressed(MouseEvent me) {
            int button = me.getButton();
            if (button == MouseEvent.BUTTON1 || button == MouseEvent.BUTTON3) {
                begin = me.getPoint();
                Point center =
                        new Point((parent.jPanel1.getVisibleRect().width -
                                currentPage.getSize().width) / 2,
                                (parent.jPanel1.getVisibleRect().height -
                                        currentPage.getSize().height) / 2);

                if (center.x < 0)
                    center.x = 0;

                if (center.y < 0)
                    center.y = 0;

                if (begin.x >= center.x && begin.x <= center.x + currentPage.getSize().width &&
                    begin.y >= center.y && begin.y <= center.y + currentPage.getSize().height) {
                    selection.width = selection.height = 0;
                    repaint();
                }
                else {
                    begin = null;
                }
            }
        }

        public void mouseReleased(MouseEvent me) {
            if (begin != null && selection.width != 0 && selection.height != 0)
            {
                begin = null;

                ImageDocument a = new ImageDocument(getSelectedSubimage(this));
                currentPage = a.getPage(1, this);
                viewPage(currentPage);

                //TODO: How to enable "prevdoc" & "recognize" buttons in Ego from here?..
                parent.parent.toggleAfterSelectionItems(true);
            }
        }

        public void mouseEntered(MouseEvent me) {
        }

        public void mouseExited(MouseEvent me) {
        }

        public void mouseDragged(MouseEvent me) {
            
            if (begin != null)
            {
                Point end = me.getPoint();
                Point center =
                    new Point((parent.jScrollPane1.getSize().width -
                    currentPage.getSize().width) / 2,
                    (parent.jScrollPane1.getSize().height -
                    currentPage.getSize().height) / 2);

                if (center.x < 0)
                    center.x = 0;

                if (center.y < 0)
                    center.y = 0;

                if (end.x <= center.x)
                    end.x = center.x;

                if (end.y <= center.y)
                    end.y = center.y;

                if (end.x > center.x + currentPage.getSize().width - 1)
                    end.x = center.x + currentPage.getSize().width - 1;

                if (end.y > center.y + currentPage.getSize().height - 1)
                    end.y = center.y + currentPage.getSize().height - 1;

                Rectangle old = new Rectangle(selection);

                selection.x = Math.min(begin.x, end.x);
                selection.y = Math.min(begin.y, end.y);
                selection.width = Math.abs(begin.x - end.x);
                selection.height = Math.abs(begin.y - end.y);

                old.add(selection);
                old.width += 1;
                old.height += 1;
                repaint(old);
            }
        }

        public void mouseMoved(MouseEvent me) {

            Point center =
                    new Point((parent.jScrollPane1.getSize().width -
                    currentPage.getSize().width) / 2,
                    (parent.jScrollPane1.getSize().height -
                    currentPage.getSize().height) / 2);

            if (center.x < 0 && center.y < 0) {
                setCursor(Cursor.getPredefinedCursor(Cursor.CROSSHAIR_CURSOR));
                return;
            }

            if (center.x < 0)
                center.x = 0;

            if (center.y < 0)
                center.y = 0;
            
            if (me.getX() >= center.x && me.getX() < center.x + currentPage.getSize().width &&
                me.getY() >= center.y && me.getY() < center.y + currentPage.getSize().height) {
                setCursor(Cursor.getPredefinedCursor(Cursor.CROSSHAIR_CURSOR));
                return;
            }

            setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }

        public void mouseWheelMoved(MouseWheelEvent e) {

            if (e.isControlDown()) {
                int clicks = e.getWheelRotation();
                
                if (clicks > 0)
                    decreaseScale();
                else
                    increaseScale();
            }

            parent.jScrollPane1.dispatchEvent(e);
        }
    }
    

    /** Creates new form DocumentPanel */
    public DocumentPanel() {
        parent = null;
        initComponents();
        viewer = (ViewerPanel)jPanel1;        
        jZoomComboBox.removeAllItems();
        jScrollPane1.getVerticalScrollBar().setUnitIncrement(20);
        toggleToolbar(jNavigateToolBar, false);
        jNavigateToolBar.setLayout(new FlowLayout(FlowLayout.CENTER, 0, 0));


        zoomComboBoxListener = new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                javax.swing.JComboBox cb = (javax.swing.JComboBox)evt.getSource();
                String selected_string = (String)cb.getSelectedItem();

                if ("comboBoxEdited".equals(evt.getActionCommand())) {
                //TODO:
                }
                else if ("comboBoxChanged".equals(evt.getActionCommand())) {
                    if (selected_string.contains("Actual size") ||
                        selected_string.contains("--")) {
                        viewer.setActualSize();
                        return;
                    }
                    if (selected_string.contains("Fit Height")) {
                        viewer.fitHeight();
                        return;
                    }
                    if (selected_string.contains("Fit Width")) {
                        viewer.fitWidth();
                        return;
                    }

                    viewer.prepPage(Double.
                            valueOf(selected_string.
                            substring(0, selected_string.length() - 1)) / 100);
                }
            }
        };

        pageNumberComboBoxListener = new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                javax.swing.JComboBox cb = (javax.swing.JComboBox)evt.getSource();
                String selected_string = (String)cb.getSelectedItem();

                if ("comboBoxEdited".equals(evt.getActionCommand())) {
                    //TODO:
                }
                else if ("comboBoxChanged".equals(evt.getActionCommand())) {
                    setPage(cb.getSelectedIndex() + 1);
                }
            }
        };
    }

    public void setParent(EgoFrame prnt) {
        parent = prnt;
    }

    private void toggleToolbar(JToolBar tb, boolean value) {
        Component[] comps = tb.getComponents();

        for (Component comp:comps) {
            comp.setEnabled(value);
        }
    }

    private void updateNavigateToolbar() {
        jZoomComboBox.removeActionListener(zoomComboBoxListener);

        if (jZoomComboBox.getItemCount() == 0)
        {
            jZoomComboBox.addItem("Actual size");
            jZoomComboBox.addItem("Fit Height");
            jZoomComboBox.addItem("Fit Width");
            jZoomComboBox.addItem("---------");
            
            for (int i = 0; i != viewer.available_scales.length; i++) {
                if (i == 0 || i == 2 || i == 4) {
                    jZoomComboBox
                        .addItem(String
                        .valueOf(viewer.available_scales[i] * 100)
                        .substring(0, 4) + "%");
                }
                else {
                jZoomComboBox
                        .addItem(String
                        .valueOf((int)(viewer.available_scales[i] * 100)) + "%");
                }
            }
        }
        jZoomComboBox.setSelectedItem(String.valueOf(viewer.getScale() * 100) + "%");

        jZoomComboBox.addActionListener(zoomComboBoxListener);

        jPageNumberComboBox.removeActionListener(pageNumberComboBoxListener);
               
        jPageNumberComboBox.removeAllItems();

        for (Integer i = 1; i <= pageCount; i++) {
            jPageNumberComboBox.addItem(i.toString() + " / " + String.valueOf(pageCount));
        }
        
        jPageNumberComboBox.addActionListener(pageNumberComboBoxListener);
    }

    public void setDocument(Document document) {        
        toggleToolbar(jNavigateToolBar, true);

        this.document = document;
        curPage = 0;
        pageCount = document.getPageCount();

        viewer.newDocument();
        viewer.initListeners();

        updateNavigateToolbar();        
                
        //TODO: check if us_2009_20090318707_a1.tif
        setPage(1);
    }

    private void setPage(int num) {
        if (num <= 0 || num > pageCount)
            return;
        
        if (num == curPage)
            return;

        curPage = num;

        DocumentPageAsImage page = document.getPage(num, viewer);
        
        viewer.viewPage(page);
        
        repaint();

        jPageNumberComboBox.setSelectedIndex(num - 1);
    }

    public void setFirstPage() {
        setPage(1);
    }

    public void setLastPage() {
        setPage(pageCount);
    }

    public void setNextPage() {
        if (curPage != pageCount) {
            setPage(curPage + 1);
        }
    }

    public void setPrevPage() {
        if (curPage != 1) {
            setPage(curPage - 1);
        }
    }

    public void increaseScale() {
        viewer.increaseScale();
    }

    public void decreaseScale() {
        viewer.decreaseScale();
    }

    public void fitHeight() {
        viewer.fitHeight();
        jZoomComboBox.setSelectedItem(
                String.valueOf((float)(viewer.getScale() * 100)) + "%");
    }

    public void fitWidth() {
        viewer.fitWidth();
        jZoomComboBox.setSelectedItem(
                String.valueOf((float)(viewer.getScale() * 100)) + "%");
    }

    public void setActualSize() {
        viewer.setActualSize();
    }

    public void setCareful() {
        careful = true;
    }


    public BufferedImage getSelectedSubimage(ImageObserver observer) {
        return viewer.getSelectedSubimage(observer);
    }

    public void recoverPreviousDocument() {
        viewer.recoverPreviousDocument();
    }

    private void initComponents() {
        jScrollPane1 = new javax.swing.JScrollPane();
        jPanel1 = new ViewerPanel(this);
        jNavigateToolBar = new javax.swing.JToolBar();
        jFirstPageButton = new javax.swing.JButton();
        jPrevPageButton = new javax.swing.JButton();
        jPageNumberComboBox = new javax.swing.JComboBox();
        jNextPageButton = new javax.swing.JButton();
        jLastPageButton = new javax.swing.JButton();
        jSeparator1 = new javax.swing.JToolBar.Separator();
        jZoomOutButton = new javax.swing.JButton();
        jZoomComboBox = new javax.swing.JComboBox();
        jZoomInButton = new javax.swing.JButton();
        jLogCheckBox = new JCheckBox("Enable log");

        setLayout(new javax.swing.BoxLayout(this, javax.swing.BoxLayout.Y_AXIS));

        jScrollPane1.setPreferredSize(new java.awt.Dimension(1004, 487));

        jPanel1.setPreferredSize(new java.awt.Dimension(100, 483));

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 559, Short.MAX_VALUE)
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 645, Short.MAX_VALUE)
        );

        jScrollPane1.setViewportView(jPanel1);

        add(jScrollPane1);

        jNavigateToolBar.setFloatable(false);
        jNavigateToolBar.setAlignmentY(0.5F);
        jNavigateToolBar.setBorderPainted(false);
        jNavigateToolBar.setMaximumSize(new java.awt.Dimension(2000, 89));

        jFirstPageButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/go-first.png")));
        jFirstPageButton.setAlignmentX(0.5F);
        jFirstPageButton.setFocusable(false);
        jFirstPageButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jFirstPageButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jFirstPageButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jFirstPageButtonActionPerformed(evt);
            }
        });
        jNavigateToolBar.add(jFirstPageButton);

        jPrevPageButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/go-previous.png")));
        jPrevPageButton.setAlignmentX(0.5F);
        jPrevPageButton.setFocusable(false);
        jPrevPageButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jPrevPageButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jPrevPageButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jPrevPageButtonActionPerformed(evt);
            }
        });
        jNavigateToolBar.add(jPrevPageButton);

        jPageNumberComboBox.setEditable(true);
        jPageNumberComboBox.setMaximumRowCount(14);
        jPageNumberComboBox.setFocusable(false);
        jPageNumberComboBox.setMaximumSize(new java.awt.Dimension(100, 27));
        jPageNumberComboBox.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jPageNumberComboBoxActionPerformed(evt);
            }
        });
        jNavigateToolBar.add(jPageNumberComboBox);

        jNextPageButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/go-next.png")));
        jNextPageButton.setAlignmentX(0.5F);
        jNextPageButton.setFocusable(false);
        jNextPageButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jNextPageButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jNextPageButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jNextPageButtonActionPerformed(evt);
            }
        });
        jNavigateToolBar.add(jNextPageButton);

        jLastPageButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/go-last.png")));
        jLastPageButton.setAlignmentX(0.5F);
        jLastPageButton.setFocusable(false);
        jLastPageButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jLastPageButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jLastPageButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jLastPageButtonActionPerformed(evt);
            }
        });
        jNavigateToolBar.add(jLastPageButton);
        jNavigateToolBar.add(jSeparator1);

        jZoomOutButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/list-remove.png")));
        jZoomOutButton.setAlignmentX(0.5F);
        jZoomOutButton.setFocusable(false);
        jZoomOutButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jZoomOutButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jZoomOutButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jZoomOutButtonActionPerformed(evt);
            }
        });
        jNavigateToolBar.add(jZoomOutButton);

        jZoomComboBox.setEditable(true);
        jZoomComboBox.setMaximumRowCount(14);
        jZoomComboBox.setFocusable(false);
        jZoomComboBox.setMaximumSize(new java.awt.Dimension(100, 27));
        jZoomComboBox.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jZoomComboBoxActionPerformed(evt);
            }
        });
        jNavigateToolBar.add(jZoomComboBox);

        jZoomInButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/list-add.png")));
        jZoomInButton.setAlignmentX(0.5F);
        jZoomInButton.setFocusable(false);
        jZoomInButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        jZoomInButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        jZoomInButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jZoomInButtonActionPerformed(evt);
            }
        });
        jNavigateToolBar.add(jZoomInButton);

        jNavigateToolBar.add(jLogCheckBox);

        add(jNavigateToolBar);
    }

    private void jFirstPageButtonActionPerformed(java.awt.event.ActionEvent evt) {
        setFirstPage();
    }

    private void jPrevPageButtonActionPerformed(java.awt.event.ActionEvent evt) {
        setPrevPage();
    }

    private void jNextPageButtonActionPerformed(java.awt.event.ActionEvent evt) {
        setNextPage();
    }

    private void jLastPageButtonActionPerformed(java.awt.event.ActionEvent evt) {
        setLastPage();
    }

    private void jZoomOutButtonActionPerformed(java.awt.event.ActionEvent evt) {
        decreaseScale();
    }

    private void jZoomInButtonActionPerformed(java.awt.event.ActionEvent evt) {
        increaseScale();
    }

    private void jPageNumberComboBoxActionPerformed(java.awt.event.ActionEvent evt) {
    }

    private void jZoomComboBoxActionPerformed(java.awt.event.ActionEvent evt) {
        
    }

    public boolean isLogEnabled() {
        return jLogCheckBox.isSelected();
    }

    public void hideLogCheckbox() {
        jLogCheckBox.setVisible(false);
    }

    private javax.swing.JButton jFirstPageButton;
    private javax.swing.JButton jLastPageButton;
    private javax.swing.JToolBar jNavigateToolBar;
    private javax.swing.JButton jNextPageButton;
    private javax.swing.JComboBox jPageNumberComboBox;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JButton jPrevPageButton;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JToolBar.Separator jSeparator1;
    private javax.swing.JComboBox jZoomComboBox;
    private javax.swing.JButton jZoomInButton;
    private javax.swing.JButton jZoomOutButton;
    private javax.swing.JCheckBox jLogCheckBox;

    private Document document;
    private ViewerPanel viewer;
    private int pageCount;
    private int curPage;
    private ActionListener zoomComboBoxListener;
    private ActionListener pageNumberComboBoxListener;
    
    private boolean careful = false;
    
    //TODO: Eeeww
    private EgoFrame parent;
}

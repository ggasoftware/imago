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

import com.ggasoftware.DocumentHandling.Document;

import java.awt.*;
import javax.swing.*;

public class EgoFrame extends EgoForm {
    
    public EgoFrame(String name) {
        super(name);        
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        new FormBuilder(this).build();
        new MenuBuilder(this).build();
        
        pack();
        
        Dimension screen_size = Toolkit.getDefaultToolkit().getScreenSize();

        setLocation((screen_size.width - getWidth()) / 2,
                    (screen_size.height - getHeight()) / 2);
        
        
        setVisible(true);
        jDocumentPanel.setParent(this);

        jMainTabbedPane.setEnabledAt(0, false);
        jMainTabbedPane.setEnabledAt(1, false);
        jMainTabbedPane.setEnabledAt(2, false);
    }
    
    public void setDocument(Document document) {
        final Document doc = document;
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                jDocumentPanel.setDocument(doc);
            }
        });

        selectTab("Document");
    }
    
    public void toggleNavigateItems(boolean value) {
        jNavigateMenu.setEnabled(value);
        jZoomMenu.setEnabled(value);
    }

    public void toggleRecognizeItems(boolean value) {
        jRecognizeButton.setEnabled(value);
        jRecognizeMenuItem.setEnabled(value);
    }

    public void toggleAfterRecognitionItems(boolean value) {
        jCopyMenuItem.setEnabled(value);
        jCopyButton.setEnabled(value);

        jSaveMenuItem.setEnabled(value);
        jSaveButton.setEnabled(value);

        jSketcherMenuItem.setEnabled(value);
        jSketcherButton.setEnabled(value);
    }

    public void toggleAfterSelectionItems(boolean value) {
        jPreviousDocumentButton.setEnabled(value);
        jPreviousDocumentMenuItem.setEnabled(value);
        
        if (value == true) {
            toggleRecognizeItems(true);
        }
    }
    
    public void showNoResultMessage() {
        JOptionPane.showMessageDialog(this, "Unfortunately, Imago couldn't recognize selected image.", "Result",
                JOptionPane.INFORMATION_MESSAGE);
    }

    public boolean setMolecule(String molecule) {
        selectTab("Molecule");
        return jMoleculePanel.setMolecule(molecule);
    }

    public void selectTab(String title) {
        int index = jMainTabbedPane.indexOfTab(title);
        jMainTabbedPane.setEnabledAt(index, true);
        jMainTabbedPane.setSelectedIndex(index);
    }

    public void enableTab(String title) {
        int index = jMainTabbedPane.indexOfTab(title);
        jMainTabbedPane.setEnabledAt(index, true);
    }
    
    public void disableTab(String title) {
        int index = jMainTabbedPane.indexOfTab(title);
        jMainTabbedPane.setEnabledAt(index, false);
    }

    public void deleteLogTab() {
        int index = jMainTabbedPane.indexOfTab("Log");
        jMainTabbedPane.removeTabAt(index);
    }
}


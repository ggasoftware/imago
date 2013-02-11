package com.ggasoftware;

import com.ggasoftware.DocumentHandling.DocumentPanel;

public class FormBuilder {
    private EgoFrame ef;
    
    public FormBuilder(EgoFrame egoComponent){
        this.ef = egoComponent;
    }
    
    public void build() {
        initToolbar();
        initTabs();
        
        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(ef.getContentPane());
        ef.getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(ef.jMainToolBar, javax.swing.GroupLayout.DEFAULT_SIZE, 727, Short.MAX_VALUE)
            .addGroup(layout.createSequentialGroup()
                .addComponent(ef.jMainTabbedPane, javax.swing.GroupLayout.DEFAULT_SIZE, 717, Short.MAX_VALUE)
                //.addContainerGap()
                )
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(ef.jMainToolBar, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                //.addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(ef.jMainTabbedPane, javax.swing.GroupLayout.DEFAULT_SIZE, 567, Short.MAX_VALUE))
        );
    }
    
    private void initToolbar() {
        ef.jMainToolBar = new javax.swing.JToolBar();
        ef.jOpenButton = new javax.swing.JButton();
        ef.jSaveButton = new javax.swing.JButton();
        ef.jPreviousDocumentButton = new javax.swing.JButton();
        ef.jRecognizeButton = new javax.swing.JButton();
        ef.jCopyButton = new javax.swing.JButton();
        ef.jSketcherButton = new javax.swing.JButton();
        
        ef.jMainToolBar.setFloatable(false);
        ef.jMainToolBar.setAlignmentX(0.0F);
        ef.jMainToolBar.setMaximumSize(new java.awt.Dimension(159, 41));

        ef.jOpenButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/document-open.png"))); 
        ef.jOpenButton.setToolTipText("Open document");
        ef.jOpenButton.setFocusable(false);
        ef.jOpenButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        ef.jOpenButton.setMaximumSize(new java.awt.Dimension(45, 43));
        ef.jOpenButton.setMinimumSize(new java.awt.Dimension(45, 43));
        ef.jOpenButton.setPreferredSize(new java.awt.Dimension(45, 43));
        ef.jOpenButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);        
        ef.jMainToolBar.add(ef.jOpenButton);

        ef.jSaveButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/document-save.png"))); 
        ef.jSaveButton.setToolTipText("Save molecule");
        ef.jSaveButton.setEnabled(false);
        ef.jSaveButton.setFocusable(false);
        ef.jSaveButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        ef.jSaveButton.setMaximumSize(new java.awt.Dimension(45, 43));
        ef.jSaveButton.setMinimumSize(new java.awt.Dimension(45, 43));
        ef.jSaveButton.setPreferredSize(new java.awt.Dimension(45, 43));
        ef.jSaveButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        ef.jMainToolBar.add(ef.jSaveButton);
        ef.jMainToolBar.addSeparator();

        ef.jPreviousDocumentButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/edit-undo.png")));
        ef.jPreviousDocumentButton.setToolTipText("Recover original document");
        ef.jPreviousDocumentButton.setEnabled(false);
        ef.jPreviousDocumentButton.setFocusable(false);
        ef.jPreviousDocumentButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        ef.jPreviousDocumentButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        ef.jMainToolBar.add(ef.jPreviousDocumentButton);
        ef.jMainToolBar.addSeparator();

        ef.jRecognizeButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/start-here.png")));
        ef.jRecognizeButton.setToolTipText("Recognize");
        ef.jRecognizeButton.setEnabled(false);
        ef.jRecognizeButton.setFocusable(false);
        ef.jRecognizeButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        ef.jRecognizeButton.setMaximumSize(new java.awt.Dimension(45, 43));
        ef.jRecognizeButton.setMinimumSize(new java.awt.Dimension(45, 43));
        ef.jRecognizeButton.setPreferredSize(new java.awt.Dimension(45, 43));
        ef.jRecognizeButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        ef.jMainToolBar.add(ef.jRecognizeButton);

        ef.jCopyButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/edit-copy.png")));
        ef.jCopyButton.setToolTipText("Copy molecule to clipboard");
        ef.jCopyButton.setEnabled(false);
        ef.jCopyButton.setFocusable(false);
        ef.jCopyButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        ef.jCopyButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        ef.jMainToolBar.add(ef.jCopyButton);

        ef.jSketcherButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/edit-find-replace.png")));
        ef.jSketcherButton.setToolTipText("Open molecule in sketcher");
        ef.jSketcherButton.setEnabled(false);
        ef.jSketcherButton.setFocusable(false);
        ef.jSketcherButton.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        ef.jSketcherButton.setVerticalTextPosition(javax.swing.SwingConstants.BOTTOM);
        ef.jMainToolBar.add(ef.jSketcherButton);
    }
    
    private void initTabs() {
        ef.jMainTabbedPane = new javax.swing.JTabbedPane();
        ef.jDocumentPanel = new DocumentPanel();
        ef.jLogScollPane = new javax.swing.JScrollPane();
        ef.logArea = new javax.swing.JTextPane();
        ef.jMoleculePanel = new MoleculePanel();

        ef.logArea.setEditable(false);
        ef.logArea.setContentType("text/html");
        ef.jLogScollPane.setViewportView(ef.logArea);

        ef.jMainTabbedPane.addTab("Document", ef.jDocumentPanel);
        ef.jMainTabbedPane.addTab("Log", ef.jLogScollPane);
        ef.jMainTabbedPane.addTab("Molecule", ef.jMoleculePanel);
    }
}

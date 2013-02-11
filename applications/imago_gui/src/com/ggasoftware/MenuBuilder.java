package com.ggasoftware;

import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.KeyStroke;

public class MenuBuilder  {
    private EgoFrame ef;
    
    public MenuBuilder(EgoFrame egoFrame) {
        this.ef = egoFrame;
    }
   
    private void buildFileMenu() {
        ef.jFileMenu = new JMenu();
        ef.jOpenMenuItem = new JMenuItem();
        ef.jQuitMenuItem = new JMenuItem();
        
        ef.jFileMenu.setMnemonic('F');
        ef.jFileMenu.setText("File");

        ef.jOpenMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_O, java.awt.event.InputEvent.CTRL_MASK));
        ef.jOpenMenuItem.setMnemonic('O');
        ef.jOpenMenuItem.setText("Open");

        ef.jFileMenu.add(ef.jOpenMenuItem);
        ef.jFileMenu.addSeparator();

        ef.jQuitMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_Q, java.awt.event.InputEvent.CTRL_MASK));
        ef.jQuitMenuItem.setMnemonic('Q');
        ef.jQuitMenuItem.setText("Quit");

        ef.jFileMenu.add(ef.jQuitMenuItem);
    }
    
    private void buildNavigateMenu() {
        ef.jNavigateMenu = new JMenu();
        ef.jFirstPageMenuItem = new JMenuItem();
        ef.jNextPageMenuItem = new JMenuItem();
        ef.jPreviousPageMenuItem = new JMenuItem();
        ef.jLastPageMenuItem = new JMenuItem();
        ef.jNavigateMenu.setMnemonic('N');
        ef.jNavigateMenu.setText("Navigate");
        ef.jNavigateMenu.setEnabled(false);

        ef.jFirstPageMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_HOME, java.awt.event.InputEvent.CTRL_MASK));
        ef.jFirstPageMenuItem.setMnemonic('F');
        ef.jFirstPageMenuItem.setText("First page");
        ef.jNavigateMenu.add(ef.jFirstPageMenuItem);

        ef.jNextPageMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_RIGHT, java.awt.event.InputEvent.CTRL_MASK));
        ef.jNextPageMenuItem.setMnemonic('N');
        ef.jNextPageMenuItem.setText("Next page");
        ef.jNavigateMenu.add(ef.jNextPageMenuItem);

        ef.jPreviousPageMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_LEFT, java.awt.event.InputEvent.CTRL_MASK));
        ef.jPreviousPageMenuItem.setMnemonic('P');
        ef.jPreviousPageMenuItem.setText("Previous page");
        ef.jNavigateMenu.add(ef.jPreviousPageMenuItem);

        ef.jLastPageMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_END, java.awt.event.InputEvent.CTRL_MASK));
        ef.jLastPageMenuItem.setMnemonic('L');
        ef.jLastPageMenuItem.setText("Last page");
        ef.jNavigateMenu.add(ef.jLastPageMenuItem);
    }
    
    private void buildZoomMenu() {
        ef.jZoomMenu = new JMenu();
        ef.jZoomInMenuItem = new JMenuItem();
        ef.jZoomOutMenuItem = new JMenuItem();
        ef.jActualSizeMenuItem = new JMenuItem();
        ef.jFitWidthMenuItem = new JMenuItem();
        ef.jFitHeightMenuItem = new JMenuItem();
        
        ef.jZoomMenu.setMnemonic('Z');
        ef.jZoomMenu.setText("Zoom");
        ef.jZoomMenu.setEnabled(false);

        ef.jZoomInMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_ADD, java.awt.event.InputEvent.CTRL_MASK));
        ef.jZoomInMenuItem.setMnemonic('I');
        ef.jZoomInMenuItem.setText("Zoom In");
        ef.jZoomMenu.add(ef.jZoomInMenuItem);

        ef.jZoomOutMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_SUBTRACT, java.awt.event.InputEvent.CTRL_MASK));
        ef.jZoomOutMenuItem.setMnemonic('O');
        ef.jZoomOutMenuItem.setText("Zoom Out");
        ef.jZoomMenu.add(ef.jZoomOutMenuItem);
        ef.jZoomMenu.addSeparator();

        ef.jActualSizeMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_1, java.awt.event.InputEvent.CTRL_MASK));
        ef.jActualSizeMenuItem.setMnemonic('A');
        ef.jActualSizeMenuItem.setText("Actual Size");
        ef.jZoomMenu.add(ef.jActualSizeMenuItem);

        ef.jFitWidthMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_2, java.awt.event.InputEvent.CTRL_MASK));
        ef.jFitWidthMenuItem.setMnemonic('W');
        ef.jFitWidthMenuItem.setText("Fit Width");
        ef.jZoomMenu.add(ef.jFitWidthMenuItem);

        ef.jFitHeightMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_3, java.awt.event.InputEvent.CTRL_MASK));
        ef.jFitHeightMenuItem.setMnemonic('H');
        ef.jFitHeightMenuItem.setText("Fit Height");
        ef.jZoomMenu.add(ef.jFitHeightMenuItem);

    }
    
    private void buildViewMenu() {
        ef.jViewMenu = new JMenu();
        ef.jViewMenu.setMnemonic('V');
        ef.jViewMenu.setText("View");
        
        buildNavigateMenu();
        ef.jViewMenu.add(ef.jNavigateMenu);
        buildZoomMenu();
        ef.jViewMenu.add(ef.jZoomMenu);
        
        ef.jPreviousDocumentMenuItem = new JMenuItem();
        ef.jPreviousDocumentMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_BACK_SPACE, java.awt.event.InputEvent.CTRL_MASK));
        ef.jPreviousDocumentMenuItem.setText("Recover original document");
        ef.jPreviousDocumentMenuItem.setEnabled(false);
        ef.jViewMenu.add(ef.jPreviousDocumentMenuItem);
    }

    private void buildMoleculeMenu() {
        ef.jMoleculeMenu = new JMenu();
        ef.jRecognizeMenuItem = new JMenuItem();
        ef.jSaveMenuItem = new JMenuItem();
        ef.jCopyMenuItem = new JMenuItem();
        ef.jSketcherMenuItem = new JMenuItem();
        
        ef.jMoleculeMenu.setMnemonic('M');
        ef.jMoleculeMenu.setText("Molecule");

        ef.jRecognizeMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_R, java.awt.event.InputEvent.CTRL_MASK));
        ef.jRecognizeMenuItem.setMnemonic('R');
        ef.jRecognizeMenuItem.setText("Recognize");
        ef.jRecognizeMenuItem.setEnabled(false);
        ef.jMoleculeMenu.add(ef.jRecognizeMenuItem);
        ef.jMoleculeMenu.addSeparator();

        ef.jSaveMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_S, java.awt.event.InputEvent.CTRL_MASK));
        ef.jSaveMenuItem.setMnemonic('S');
        ef.jSaveMenuItem.setText("Save");
        ef.jSaveMenuItem.setEnabled(false);
        ef.jMoleculeMenu.add(ef.jSaveMenuItem);

        ef.jCopyMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_C, java.awt.event.InputEvent.CTRL_MASK));
        ef.jCopyMenuItem.setMnemonic('C');
        ef.jCopyMenuItem.setText("Copy to clipboard");
        ef.jCopyMenuItem.setEnabled(false);
        ef.jMoleculeMenu.add(ef.jCopyMenuItem);

        ef.jSketcherMenuItem.setAccelerator(KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_M, java.awt.event.InputEvent.CTRL_MASK));
        ef.jSketcherMenuItem.setMnemonic('M');
        ef.jSketcherMenuItem.setText("Open in MarvinSketch");
        ef.jSketcherMenuItem.setEnabled(false);
        ef.jMoleculeMenu.add(ef.jSketcherMenuItem);
    }
    
    public void build() {
        ef.jMainMenuBar = new JMenuBar();
        
        buildFileMenu();
        buildViewMenu();
        buildMoleculeMenu();
        
        ef.jMainMenuBar.add(ef.jFileMenu);
        ef.jMainMenuBar.add(ef.jViewMenu);
        ef.jMainMenuBar.add(ef.jMoleculeMenu);
        ef.setJMenuBar(ef.jMainMenuBar);
    }
}

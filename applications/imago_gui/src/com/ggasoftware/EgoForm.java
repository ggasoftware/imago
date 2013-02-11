package com.ggasoftware;

import com.ggasoftware.DocumentHandling.DocumentPanel;
import javax.swing.JFrame;

public class EgoForm extends JFrame {
    public EgoForm(String name) {
        super(name);
    }
    
    protected javax.swing.JToolBar jMainToolBar;
    protected javax.swing.JButton jOpenButton;
    protected javax.swing.JButton jPreviousDocumentButton;
    protected javax.swing.JButton jRecognizeButton;
    protected javax.swing.JButton jSaveButton;
    protected javax.swing.JButton jSketcherButton;
    protected javax.swing.JButton jCopyButton;
    
    protected javax.swing.JTabbedPane jMainTabbedPane;    
    protected DocumentPanel jDocumentPanel;
    protected MoleculePanel jMoleculePanel;
    protected javax.swing.JScrollPane jLogScollPane;
    protected javax.swing.JTextPane logArea;
    
    protected javax.swing.JMenuBar jMainMenuBar;
    protected javax.swing.JMenuItem jActualSizeMenuItem;
    protected javax.swing.JMenuItem jCopyMenuItem;
    
    protected javax.swing.JMenu jFileMenu;
    protected javax.swing.JMenuItem jFirstPageMenuItem;
    protected javax.swing.JMenuItem jFitHeightMenuItem;
    protected javax.swing.JMenuItem jFitWidthMenuItem;
    protected javax.swing.JMenuItem jLastPageMenuItem;
    
    protected javax.swing.JMenu jMoleculeMenu;
    protected javax.swing.JMenu jNavigateMenu;
    protected javax.swing.JMenuItem jNextPageMenuItem;
    protected javax.swing.JMenuItem jOpenMenuItem;
    
    protected javax.swing.JMenuItem jPreviousDocumentMenuItem;
    protected javax.swing.JMenuItem jPreviousPageMenuItem;
    protected javax.swing.JMenuItem jQuitMenuItem;
    protected javax.swing.JMenuItem jRecognizeMenuItem;
    protected javax.swing.JMenuItem jSaveMenuItem;
    protected javax.swing.JMenuItem jSketcherMenuItem;
    
    protected javax.swing.JMenu jViewMenu;
    
    protected javax.swing.JMenu jZoomMenu;
    protected javax.swing.JMenuItem jZoomOutMenuItem;
    protected javax.swing.JMenuItem jZoomInMenuItem;
}

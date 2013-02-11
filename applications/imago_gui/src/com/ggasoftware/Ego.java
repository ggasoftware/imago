package com.ggasoftware;

import com.ggasoftware.DocumentHandling.Document;
import com.ggasoftware.DocumentHandling.ImageDocument;
import com.ggasoftware.DocumentHandling.PdfDocument;
import com.ggasoftware.DocumentHandling.TiffDocument;
import com.ggasoftware.imago.Imago;
import com.ggasoftware.imago.ImagoException;
import java.awt.Dialog;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.Transferable;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.image.BufferedImage;
import java.io.*;
import java.net.MalformedURLException;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.UIManager;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.text.StyledDocument;
import javax.swing.text.html.HTMLDocument;

public class Ego {
    private EgoFrame frame;
    private File curFile;
    private Imago imago = new Imago();
    private String molecule = "";
    private String prevSaveDirectory = ".";
    private String logTempDir = null;
    private OpenSketcherDialog osd;

    public Ego(String filename) {
        frame = new EgoFrame("Ego");
        setActions();
        if (filename != null) {
            setFile(new File(filename));
        }

        try {
            createTempDirectory();
        } catch (IOException e) {
            frame.jDocumentPanel.hideLogCheckbox();
            frame.deleteLogTab();
        }

        osd = new OpenSketcherDialog(frame);
        frame.addWindowListener(new WindowListener() {
            public void windowOpened(WindowEvent e) {}
            public void windowClosing(WindowEvent e) {
                dropLogImages();
                
                //Deleting tempDirectory
                if (logTempDir != null)
                    new File(logTempDir).delete();
            }
            public void windowClosed(WindowEvent e) {}
            public void windowIconified(WindowEvent e) {}
            public void windowDeiconified(WindowEvent e) {}
            public void windowActivated(WindowEvent e) {}
            public void windowDeactivated(WindowEvent e) {}
        });
    }

    public JFrame getJFrame() {
        return (JFrame)frame;
    }
    
    public void setFile(File file) {
        if (file == curFile) {
            return;
        }

        if (!Utils.checkFile(file)) {
            return;
        }

        curFile = file;
        Document doc;
        try {
            if (Utils.isPDF(file)) {
                doc = new PdfDocument(file);
            } else if (Utils.isTIF(file)) {
                doc = new TiffDocument(file);
            } else if (Utils.isAcceptableImage(file)) {
                doc = new ImageDocument(file);
            } else {
                return;
            }
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }

        frame.toggleAfterRecognitionItems(false);
        frame.toggleAfterSelectionItems(false);
        frame.toggleNavigateItems(true);
        frame.setTitle("Ego: " + file.getName());
        frame.toggleRecognizeItems(true);
        frame.jDocumentPanel.setCareful();

        frame.setDocument(doc);
    }

    public void setFileFromDialog() {
        String path = ".";
        if (curFile != null) {
            path = curFile.getParent();
        }
        JFileChooser jfc = new JFileChooser(path);
        jfc.setDialogTitle("Select document");
        jfc.setFileFilter(new FileNameExtensionFilter("Documents (png, jpg, gif"
                + ", pdf, tif, tiff)", "png", "jpg", "gif", "pdf", "tif", "tiff"));
        jfc.setFileSelectionMode(JFileChooser.FILES_ONLY);
        if (jfc.showOpenDialog(frame) == JFileChooser.APPROVE_OPTION) {
            setFile(jfc.getSelectedFile());
        }
    }

    private boolean recognitionThread() {
        BufferedImage img = frame.jDocumentPanel.getSelectedSubimage(null);
        boolean logEnabled = frame.jDocumentPanel.isLogEnabled();
        boolean result = true;

        if (logTempDir != null)
            frame.disableTab("Log");

        try {
            if (logEnabled) {
                imago.enableLog(true);
            }
            else
                imago.disableLog();

            imago.loadImage(img);
            imago.filterImage();
            imago.recognize();
            molecule = imago.getResultMolecule();
        } catch (ImagoException ex) {
            ex.printStackTrace();
            result = false;
        }

        if (logEnabled)
            logThread();

        return result;
    }

    private void createTempDirectory() throws IOException {
        File tmpfile = File.createTempFile("imago", null);
        String tmpdir_path = tmpfile.getAbsolutePath() + ".d";
        tmpfile.delete();
        final File tmpdir = new File(tmpdir_path);

        if (tmpdir.mkdir()) {
            logTempDir = tmpdir.getAbsolutePath();
        } else {
            throw new IOException("Cannot create temp diretory");
        }
    }

    private void dropLogImages() {
        if (logTempDir == null)
            return;

        String imgsPath = logTempDir + File.separator + "htmlimgs";
        File f = new File(imgsPath);
        if (f.exists()) {
            for (String child : f.list()) {
                new File(imgsPath, child).delete();
            }
        }
        f.delete();
    }

    public void logThread() {
        assert(logTempDir != null);

        Imago.LogRecord[] log = imago.getLogRecords();

        dropLogImages();
        new File(logTempDir + File.separator + "htmlimgs").mkdir();
        for (int i = 1; i < log.length; ++i) {
            try {
                FileOutputStream fos = new FileOutputStream(new File(logTempDir + File.separator + log[i].filename));
                fos.write(log[i].data);
                fos.close();
            } catch (IOException ex) {
            }
        }

        frame.logArea.setText(new String(log[0].data));
        StyledDocument sd = frame.logArea.getStyledDocument();
        if (sd instanceof HTMLDocument) {
            try {
                ((HTMLDocument) sd).setBase(new File(logTempDir).toURI().toURL());
            } catch (MalformedURLException e) {
            }
        }
        frame.enableTab("Log");
    }

    public void recognize() {
        final JDialog waitingDialog = new JDialog(frame, "Please wait",
                Dialog.ModalityType.DOCUMENT_MODAL);
        
        waitingDialog.setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
        waitingDialog.add(new WaitingPanel());
        waitingDialog.pack();
        Point loc = frame.getLocation();
        Dimension fdim = frame.getSize(),
                  ddim = waitingDialog.getSize();
        loc.x = loc.x + fdim.width / 2 - ddim.width / 2;
        loc.y = loc.y + fdim.height / 2 - ddim.height / 2;
        waitingDialog.setLocation(loc);
        
        Thread t = new Thread(new Runnable() {
            public void run() {
                if (!recognitionThread()
                        || !frame.setMolecule(molecule)) {
                    frame.showNoResultMessage();
                }

                frame.toggleAfterRecognitionItems(true);
                waitingDialog.dispose();
            }
        });
        t.start();
        waitingDialog.setVisible(true);      
    }    

    public void saveMolecule() {
        JFileChooser jfc = new JFileChooser(prevSaveDirectory);
        jfc.setFileFilter(new javax.swing.filechooser.FileNameExtensionFilter("Molfile", "mol"));
        int res = jfc.showSaveDialog(frame);

        if (res == JFileChooser.APPROVE_OPTION) {
            File file = jfc.getSelectedFile();
            prevSaveDirectory = file.getParent();

            try {
                FileWriter fw = new FileWriter(file);
                fw.write(molecule);
                fw.flush();
                fw.close();
            } catch (IOException e) {
            }
        }

    }

    public void copyToClipboard() {
        Clipboard systemClipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
        Transferable a = systemClipboard.getContents(null);
        Transferable transferableMolecule = new MoleculeSelection(molecule);

        try {
            systemClipboard.setContents(transferableMolecule, null);
        }
        catch (Exception e) {
            System.out.print(e.getMessage());
        }

        a = systemClipboard.getContents(null);
    }
    
    public final void setActions() {
        ActionListener openAction = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                setFileFromDialog();
            }
        };

        ActionListener recognizeAction = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                recognize();
            }
        };
        ActionListener saveAction = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                saveMolecule();
            }
        };
        ActionListener copyAction = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                copyToClipboard();
            }
        };

        ActionListener previousDocumentAction = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                frame.jDocumentPanel.recoverPreviousDocument();
                frame.jMainTabbedPane.setSelectedIndex(0);
            }
        };

        ActionListener sketcherAction = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    osd.show(molecule);
                } catch (java.lang.UnsatisfiedLinkError ex) {
                }
            }
        };

        frame.jOpenButton.addActionListener(openAction);
        frame.jRecognizeButton.addActionListener(recognizeAction);
        frame.jSaveButton.addActionListener(saveAction);
        frame.jCopyButton.addActionListener(copyAction);
        frame.jPreviousDocumentButton.addActionListener(previousDocumentAction);
        frame.jSketcherButton.addActionListener(sketcherAction);

        frame.jQuitMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                getJFrame().dispose();
            }
        });

        frame.jOpenMenuItem.addActionListener(openAction);
        frame.jRecognizeMenuItem.addActionListener(recognizeAction);
        frame.jSaveMenuItem.addActionListener(saveAction);
        frame.jCopyMenuItem.addActionListener(copyAction);

        frame.jFirstPageMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                frame.jDocumentPanel.setFirstPage();
            }
        });

        frame.jNextPageMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                frame.jDocumentPanel.setNextPage();
            }
        });
        frame.jPreviousPageMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                frame.jDocumentPanel.setPrevPage();
            }
        });

        frame.jLastPageMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                frame.jDocumentPanel.setLastPage();
            }
        });

        frame.jZoomInMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                frame.jDocumentPanel.increaseScale();
            }
        });

        frame.jZoomOutMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                frame.jDocumentPanel.decreaseScale();
            }
        });

        frame.jActualSizeMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                frame.jDocumentPanel.setActualSize();
            }
        });

        frame.jFitWidthMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                frame.jDocumentPanel.fitWidth();
            }
        });

        frame.jFitHeightMenuItem.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                frame.jDocumentPanel.fitHeight();
            }
        });

        frame.jPreviousDocumentMenuItem.addActionListener(previousDocumentAction);
        frame.jSketcherMenuItem.addActionListener(sketcherAction);
    }

    private static void setupUI() {
        try {
            String os = System.getProperty("os.name");
            if (os.indexOf("Linux") >= 0) {
                UIManager.LookAndFeelInfo infos[] = UIManager.getInstalledLookAndFeels();

                for (int i = 0; i != infos.length; i++) {
                    if (infos[i].getName().equals("Nimbus")) {
                        UIManager.setLookAndFeel(infos[i].getClassName());
                        return;
                    }
                }
            }

            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
        }
    }
    
    public static void main(String args[]) {
        setupUI();

        String filename = null;
        if (args.length > 0) {
            filename = args[0];
        }

        new Ego(filename);
    }
}

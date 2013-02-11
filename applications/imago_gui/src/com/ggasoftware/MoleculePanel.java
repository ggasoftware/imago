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

import com.ggasoftware.indigo.*;
import java.awt.BorderLayout;
import java.awt.Image;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import javax.imageio.ImageIO;
import javax.swing.JPanel;

public class MoleculePanel extends JPanel {
    private ImagePanel molPicture = new ImagePanel();
    private Indigo indigo;
    private IndigoRenderer indigo_renderer;

    private String molecule = "";
    public MoleculePanel() {
        indigo = new Indigo();
        indigo_renderer = new IndigoRenderer(indigo);
        indigo.setOption("render-output-format", "png");
        indigo.setOption("render-label-mode", "terminal-hetero");
        indigo.setOption("render-coloring", true);
        indigo.setOption("ignore-stereochemistry-errors", true);

        setLayout(new BorderLayout());
        add(molPicture);

        molPicture.addComponentListener(new ComponentListener() {
            @Override
            public void componentResized(ComponentEvent ce) {
                String params = ce.paramString();
                int b = params.lastIndexOf(" ");
                int e = params.lastIndexOf(")");
                params = params.substring(b + 1, e);
                String[] dims = params.split("x");
                int width = Integer.parseInt(dims[0]);
                int height = Integer.parseInt(dims[1]);
                indigo.setOption("render-image-size", width, height);
            }

            @Override
            public void componentMoved(ComponentEvent ce) {}
            @Override
            public void componentShown(ComponentEvent ce) {}
            @Override
            public void componentHidden(ComponentEvent ce) {}
        });
    }

    public void clear() {
        molecule = null;
        molPicture.setImage(null);
    }

    public String getMoleculeString() {
        return molecule;
    }
    
    public boolean setMolecule(String molecule) {
        this.molecule = molecule;
        indigo.setOption("render-background-color", 1.0f, 1.0f, 1.0f);
        IndigoObject indigo_mol = indigo.loadQueryMolecule(molecule);

        byte[] pict = indigo_renderer.renderToBuffer(indigo_mol);
        try {
            Image mol_image = ImageIO.read(new ByteArrayInputStream(pict));

            molPicture.setImage(mol_image);
            molPicture.repaint();
        } catch (IOException ex) {
            return false;
        }
        return true;
    }
}
/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
 *
 * This file is part of Imago OCR project.
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

import javax.swing.JLabel;
import javax.swing.JPanel;

public class WaitingPanel extends JPanel {
    public WaitingPanel() {
        super(true);
        setSize(150, 50);
        add(new JLabel("Recognizing..."));
    }
}

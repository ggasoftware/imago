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

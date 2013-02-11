/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.ggasoftware;

import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.ClipboardOwner;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.SystemFlavorMap;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

/**
 * A Transferable which implements the capability required
 * to transfer a Molecule (in MDL MolFile format) in Windows clipboard.
 */
public class MoleculeSelection implements Transferable, ClipboardOwner {
        
        public MoleculeSelection(String mol) {
            try {
                extra_flavor = new DataFlavor("chemical/x-mdl-molfile");
            }
            catch(ClassNotFoundException classnotfoundexception) {}

            String os = System.getProperty("os.name").toLowerCase();

            if (os.indexOf("win") >= 0) {
                registerNativeNames("MDLCT");
            }
            else if (os.indexOf("mac") >= 0) {
                registerNativeNames("swsC");
            }
            
            molecule_mol = new String();
            molecule_mol = mol;

            molecule_mdlct = new String();

            ByteArrayOutputStream bytearrayoutputstream = new ByteArrayOutputStream();
            int i = 0;
            for (int j = 0; j < mol.length(); j++) {
                char c = mol.charAt(j);
                
                if (c != '\n' && c != '\r') {
                    continue;
                }
                
                bytearrayoutputstream.write(j - i);

                for (int l = i; l < j; l++) {
                    bytearrayoutputstream.write(mol.charAt(l));
                }

                if (j < mol.length() - 1) {
                    char c1 = mol.charAt(j + 1);

                    if ((c1 == '\n' || c1 == '\r') && c1 != c) {
                        j++;
                    }
                }
                i = j + 1;
            }

            if (i < mol.length()) {
                bytearrayoutputstream.write(mol.length() - i);

                for (int k = i; k < mol.length(); k++) {
                    bytearrayoutputstream.write(mol.charAt(k));
                }
            }

            molecule_mdlct = bytearrayoutputstream.toString();
        }

        public final void registerNativeNames(String name) {
            SystemFlavorMap systemflavormap = (SystemFlavorMap)SystemFlavorMap.getDefaultFlavorMap();
            
            systemflavormap.addUnencodedNativeForFlavor(extra_flavor, name);
            systemflavormap.addFlavorForUnencodedNative(name, extra_flavor);
        }

        public DataFlavor[] getTransferDataFlavors() {
            DataFlavor[] new_ar = new DataFlavor[2];

            new_ar[0] = data_flavor[0];
            new_ar[1] = extra_flavor;
            return new_ar;
        }

        public boolean isDataFlavorSupported(DataFlavor flavor) {

            if (!flavor.equals(data_flavor[0]) && !flavor.equals(extra_flavor)) {
                return false;
            }
            return true;
        }

        public Object getTransferData(DataFlavor flavor)
                throws UnsupportedFlavorException, IOException {
            if (flavor.equals(data_flavor[0])) {
                return molecule_mol;
            }
            else {
                return new ByteArrayInputStream(molecule_mdlct.getBytes());
            }
	} 

        public void lostOwnership(Clipboard clipboard, Transferable contents) {
            //throw new UnsupportedOperationException("Not supported yet.");
        }
        
        private String molecule_mol, molecule_mdlct;
        private DataFlavor data_flavor[] =
               new DataFlavor[]{ DataFlavor.stringFlavor };

        private DataFlavor extra_flavor;
    }

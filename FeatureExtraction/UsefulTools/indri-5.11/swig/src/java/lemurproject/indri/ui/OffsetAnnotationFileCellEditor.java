/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package lemurproject.indri.ui;

import java.awt.event.ActionEvent;
import javax.swing.*;

/**
 *
 * @author mhoy
 */
public class OffsetAnnotationFileCellEditor extends AbstractCellEditor implements javax.swing.table.TableCellEditor {
  
  private JPanel pnlEditor;
  private JTextField txtValue;
  private JButton btnBrowse;
 
  public OffsetAnnotationFileCellEditor() {
    pnlEditor=new JPanel();
    pnlEditor.setLayout(new java.awt.BorderLayout());
    txtValue=new JTextField();
    pnlEditor.add(txtValue, java.awt.BorderLayout.CENTER);
    btnBrowse=new JButton("...");
    btnBrowse.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        browseForFile();
      }
    });
    pnlEditor.add(btnBrowse, java.awt.BorderLayout.EAST);
  }
      
  protected void browseForFile() {
    JFileChooser fileChooser=new JFileChooser();
    fileChooser.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
    fileChooser.setMultiSelectionEnabled(false);
    int retVal=fileChooser.showOpenDialog(pnlEditor);
    if (retVal==JFileChooser.APPROVE_OPTION) {
      java.io.File selectedFile=fileChooser.getSelectedFile();
      String fullPath=selectedFile.getAbsolutePath();

      // if user double clicked a directory to select,
      // we get the directory name as the selected file
      // in the intended directory.
      // so check that the file exists and is a directory.
      // if not, try the parent directory.
      if (!selectedFile.exists()) {
        java.io.File parentFile=selectedFile.getParentFile();
        if (parentFile!=null) {
          fullPath=parentFile.getAbsolutePath();
        }
      } // end if (!selectedFile.exists())
      txtValue.setText(fullPath);
    } // end if (retVal==JFileChooser.APPROVE_OPTION)
  }
  
  public java.awt.Component getTableCellEditorComponent(JTable table, Object value, boolean isSelected, int rowIndex, int vColIndex) {
    txtValue.setText((String)value);
    return pnlEditor;
  }
  
  public Object getCellEditorValue() {
    return txtValue.getText();
  }
}

package lemurproject.indri.ui;

import javax.swing.table.AbstractTableModel;

public class OffsetAnnotationTableModel extends AbstractTableModel {

	private static  String[] offsetColumnNames= {"Data File", "Offset Annotation File" };

	private java.util.Vector fileNames;
	private java.util.Vector offsetFileNames;
	
	public OffsetAnnotationTableModel() {
		fileNames=new java.util.Vector();
		offsetFileNames=new java.util.Vector();
	}
	
	public String getColumnName(int col) {
	  	return offsetColumnNames[col];
	}
	
	public boolean containsFilename(String filename) {
		return fileNames.contains(filename);
	}
	
	public void setFileNames(java.util.Vector fileVector) {
		fileNames.clear();
		offsetFileNames.clear();
		fileNames.addAll(fileVector);
		for (int i=0; i < fileNames.size(); i++) {
			offsetFileNames.add("");
		}
		this.fireTableDataChanged();
	}
	
	public void addFilename(String newFilename) {
    // only add if the filename does not already exist...
    if (!fileNames.contains(newFilename)) {
      fileNames.add(newFilename);
      offsetFileNames.add("");
      this.fireTableDataChanged();
    }
	}
	
	public void removeFilename(String oldFilename) {
		int whichIndex=fileNames.indexOf(oldFilename);
		if (whichIndex > -1) {
			fileNames.remove(whichIndex);
			offsetFileNames.remove(whichIndex);
		}
	}
	
	public int getColumnCount() {
		return 2;
	}

	public int getRowCount() {
		return fileNames.size();
	}
	
	public boolean isCellEditable(int row, int column) {
		if ((row < 0) || (row > (fileNames.size()-1))) { return false; }
		if (column != 1) { return false; }
		return true;
	}
	
	public void setValueAt(Object value, int row, int column) {
		if ((row < 0) || (row > (fileNames.size()-1))) { return; }
		if (column != 1) { return; }
		offsetFileNames.setElementAt(value.toString(), row);
	}
	
	public java.util.HashMap getAllValues() {
		java.util.HashMap retVal=new java.util.HashMap();
		int nSize=fileNames.size();
		for (int i=0; i < nSize; i++) {
			String thisFile=(String)fileNames.get(i);
			String thisOffsetFile=(String)offsetFileNames.get(i);
			retVal.put(thisFile.trim(), thisOffsetFile.trim());
		}
		return retVal;
	}

	public Object getValueAt(int rowIndex, int columnIndex) {
		if ((rowIndex < 0) || (rowIndex > (fileNames.size()-1))) { return null; }
		if ((columnIndex < 0) || (columnIndex > 1)) { return null; }
		if (columnIndex==0) {
			return fileNames.get(rowIndex);
		} else {
			return offsetFileNames.get(rowIndex);
		}
	}

}

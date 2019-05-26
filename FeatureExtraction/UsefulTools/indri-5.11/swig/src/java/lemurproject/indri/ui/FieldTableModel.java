package lemurproject.indri.ui;

import javax.swing.table.AbstractTableModel;

public class FieldTableModel extends AbstractTableModel {

	private static  String[] fieldColumnNames= {"Field Name", "Is Numeric?" };

	java.util.Vector fieldNames;
	java.util.Vector isNumeric;
  
  javax.swing.table.TableColumnModel columnHeader=null;
  
	public FieldTableModel() {
		fieldNames=new java.util.Vector();
		isNumeric=new java.util.Vector();
    columnHeader=new javax.swing.table.DefaultTableColumnModel();
	}
	
	public String getColumnName(int col) {
	  	return fieldColumnNames[col];
	}
	
	public int getColumnCount() {
		return 2;
	}

	public int getRowCount() {
		return fieldNames.size();
	}
	
	public void addNewField() {
		fieldNames.add("");
		isNumeric.add(new Boolean(false));
		this.fireTableDataChanged();
	}
	
	public void removeField(int row) {
		if ((row < 0) || (row > (fieldNames.size()-1))) { return; }
		fieldNames.remove(row);
		isNumeric.remove(row);
		this.fireTableDataChanged();
	}
	
	public boolean isCellEditable(int row, int column) {
		return true;
	}
	
	public Class getColumnClass(int columnIndex) {
		if (columnIndex==1) { return Boolean.class; }
		return String.class;
	}
	
	public void setValueAt(Object value, int row, int column) {
		if ((row < 0) || (row > (fieldNames.size()-1))) { return; }
		if (column == 0) { 
			fieldNames.setElementAt(value.toString(), row);
		} else {
			isNumeric.setElementAt(new Boolean(((Boolean)value).booleanValue()), row);
		}
	}

	public Object getValueAt(int rowIndex, int columnIndex) {
		if ((rowIndex < 0) || (rowIndex > (fieldNames.size()-1))) { return null; }
		if ((columnIndex < 0) || (columnIndex > 1)) { return null; }
		if (columnIndex==0) {
			return fieldNames.get(rowIndex);
		} else {
			return isNumeric.get(rowIndex);
		}
	}
 
}



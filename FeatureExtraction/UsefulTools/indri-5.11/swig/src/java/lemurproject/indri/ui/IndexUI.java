package lemurproject.indri.ui;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import lemurproject.indri.*;

/**
   User interface for building Indri indexes.
   @author David Fisher
   @author Mark J. Hoy
   @version 1.1
  
   12/3/07 - added fields tab (MJH)
*/
public class IndexUI extends JPanel implements ActionListener, 
					       ItemListener, CaretListener, TableModelListener
{
    /** Status line messages visible for all tabs. */
    private JLabel status;
    /** Messages pane for prog output */
    private JTextArea messages;
    /** Top level container */
    private JTabbedPane tabbedPane;
    /** Single choice */
    private JCheckBox doRecurse, doStem;
    /** Multiple choices */
    private JComboBox stemmers,	memoryLim, docFormat;
    /** Action Buttons */
    private JButton browse, cfbrowse, stopBrowse, cfRemove, go, stop;
    /** input/browseable filename */
    private JTextField iname, stopwordlist;
    /** field names */
    private JTextField colFields; //, indFields; // indFields now replaced with fields tab...
	
    /** filename filter */
    private JTextField filterString;
    /** browse multiple filenames */
    private JList collectionFiles;
    /** hold the file names */
    private DefaultListModel cfModel;
    /** File chooser starting in current directory */
    private final JFileChooser fc = 
	new JFileChooser(System.getProperties().getProperty("user.dir"));
    /** data directory paths */    
    private String stopwords;
    /** Help file for the application */
    private final static String helpFile = "properties/IndriIndex.html";
    /** The little icon */
    //    private final static String iconFile = "properties/lemur_icon.GIF";
    //    private final static String iconFile = "properties/lemur.GIF";
    private final static String iconFile = "properties/lemur_head_32.gif";
    /** The big logo */
    private final static String logoFile = null;
    /** Indri FileClassEnvironments */
    private final static String [] formats = {"trecweb", "trectext", "html",
					      "doc", "ppt", "pdf", "txt"};
    /** Memory limit choices */
    private final static String [] lims = {"  64 MB", "  96 MB", " 128 MB", 
					   " 256 MB", " 512 MB", " 768 MB","1024 MB"};
    /** Stemmer types */
    private final static String[] sTypes = {"krovetz", "porter"};
	
    /** are we appending? */
    boolean appendIndex = false;
	
    /** MenuBar */
    private JMenuBar menuBar;
    /** Menus */
    private JMenu fileMenu, helpMenu;
    /** Menu Items */
    private JMenuItem fOpen, fSave, fPrefs, fQuit, hHelp, hAbout;
    /** The Indri Icon. */
    private ImageIcon indriIcon;
    /** About the indexer. */
    private final static String aboutText = "Indri Indexer UI 1.0\n" +
	"Copyright (c) 2004 University of Massachusetts";
    /** Frame for help window */
    private JFrame helpFrame;
	
    /** Fields and Metadata items */
    private JPanel indexFieldPanel;
    private JTable fieldTable;
    private FieldTableModel fieldTableModel;
    private JButton btnAddField;
    private JButton btnRemoveField;
    private JTable offsetAnnotationFileTable;
    private OffsetAnnotationTableModel offsetAnnotationFilesTableModel;
    private JTextField txtPathToHarvestLinks;
    private JButton btnHarvestLinks;    
    
    private static String[] fieldColumnTooltips={"The field name to index", "Is the field numeric?"};
    private static String[] annotationsColumnTooltips={"The datafile for the annotations", "The path to the annotation file(s)"};
    
    // for processing offset files while indexing...
    private Vector dataFilesOffsetFiles=null;
    
    /** Get the ball rolling. */
    public IndexUI () {	
	super(new BorderLayout());
	initGUI();
    }
    /**
     * Initialize the GUI elements, including preloading the
     * help frame.
     */
    private void initGUI() {	
	// starting with a JPanel using BorderLayout
	// indexing tab to use GridBagLayout
	// reuse for each labeled component.
	JLabel label;
	// set up icon images
	indriIcon = createImageIcon(iconFile);
	ImageIcon icon = null;  // no icon on tabs
	// initialize help
	makeHelp();
	tabbedPane = new JTabbedPane();
	// initialize the indexing tab
		
	JComponent panel = new JPanel();
	GridBagLayout layout = new GridBagLayout();
	GridBagConstraints constraints = new GridBagConstraints();
	constraints.anchor = GridBagConstraints.LINE_END;
	panel.setLayout(layout);	
		
	// browse button for index name
	browse = new JButton("Browse...");
	browse.addActionListener(this);
	browse.setToolTipText("Browse to a directory and enter a basename " +
			      "for the index");
	// index name
	iname = new JTextField("", 25);
	iname.setToolTipText("Enter a basename for the index or browse to " +
			     "a directory");
	iname.addCaretListener(this);
	label = new JLabel("Index Name: ", JLabel.TRAILING);
	label.setLabelFor(iname);
	constraints.insets = new Insets(10,10,0,0);
	constraints.gridx = 0;
	constraints.gridy = 0;
	panel.add(label, constraints);
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(iname, constraints);
	constraints.gridx = 2;
	panel.add(browse, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
	// data files list
	cfModel = new DefaultListModel();
	collectionFiles = new JList(cfModel);
	collectionFiles.setSelectionMode(ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
	collectionFiles.setVisibleRowCount(5);
	collectionFiles.setToolTipText("Browse to a directory and select " + 
				       "input files or directories.");
	JScrollPane listScrollPane = new JScrollPane(collectionFiles);
	listScrollPane.setPreferredSize(new Dimension(400, 100));
	// browse button for data files
	cfbrowse = new JButton("Browse...");
	cfbrowse.addActionListener(this);
	cfbrowse.setToolTipText("Browse to a directory and select input " + 
				"files or directories.");
	// remove buttone for data files
	cfRemove = new JButton("Remove");
	cfRemove.addActionListener(this);
	cfRemove.setToolTipText("Remove selected files from the list.");
	// second row
	constraints.gridy = 1;
	label = new JLabel("Data File(s): ", JLabel.TRAILING);
	constraints.gridx = 0;
	panel.add(label, constraints);
	constraints.gridx = 1;
	panel.add(listScrollPane, constraints);
	// check box for recurse into subdirectories
	doRecurse = new JCheckBox("Recurse into subdirectories");
	doRecurse.setToolTipText("<html>When checked and a directory is in the<br>" + 
				 "data files list, recursively add all<br>" +
				 "data files in that directory and all of<br>" + 
				 "its subdirectories into the data<br>" +
				 "files list.</html>");
	JPanel cfButtons = new JPanel(new BorderLayout());
	cfButtons.add(cfbrowse, BorderLayout.NORTH);
	// button layout needs some spacing, bleah. Gridbag it too?
	cfButtons.add(cfRemove, BorderLayout.SOUTH);
	constraints.gridx = 2;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(cfButtons, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
	// third row
	constraints.gridy = 2;
	label = new JLabel("Filename filter: ", JLabel.TRAILING);
	constraints.gridx = 0;
	panel.add(label, constraints);
	filterString = new JTextField("", 25);
	label.setLabelFor(filterString);
	filterString.setToolTipText("Specify a filename filter, eg *.sgml");
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(filterString, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
	constraints.gridx = 2;
	panel.add(doRecurse, constraints);
		
	// fourth row
	constraints.gridy = 3;
		
	colFields = new JTextField("docno", 25);
	colFields.setToolTipText("<html>Comma delimited list of field names,<br>" +
				 "without spaces to index as metadata.</html>");
		
//	indFields = new JTextField("title", 25);
//	indFields.setToolTipText("Comma delimited list of field names, " +
//				 "without spaces to index as data for " +
//				 "field queries");
	label = new JLabel("Collection Fields: ", JLabel.TRAILING);
	label.setLabelFor(colFields);
	constraints.gridx = 0;
	panel.add(label, constraints);
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(colFields, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
		
	// fifth row
	constraints.gridy = 4;
		
//	label = new JLabel("Indexed Fields: ", JLabel.TRAILING);
//	label.setLabelFor(colFields);
//	constraints.gridx = 0;
//	panel.add(label, constraints);
//	constraints.gridx = 1;
//	constraints.anchor = GridBagConstraints.LINE_START;
//	panel.add(indFields, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
		
	// sixth row
	constraints.gridy = 5;
	docFormat = new JComboBox(formats);
	docFormat.setToolTipText("Select format of input files");	
	docFormat.addActionListener(this);
	label = new JLabel("Document format: ", JLabel.TRAILING);
	constraints.gridx = 0;
	panel.add(label, constraints);
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(docFormat, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
	// seventh row
	constraints.gridy = 6;
		
	stopwordlist = new JTextField(stopwords, 25);
	stopwordlist.setToolTipText("<html>Enter a stopword list or browse to<br>" +
				    "select. Clear this field if you do<br>" +
				    "not want to stop this index.</html>");
	stopBrowse = new JButton("Browse...");
	stopBrowse.addActionListener(this);
	stopBrowse.setToolTipText("Browse to a directory and select " +
				  "a stoplist.");
	label = new JLabel("Stopword list: ", JLabel.TRAILING);
	constraints.gridx = 0;
	panel.add(label, constraints);
		
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(stopwordlist, constraints);
	constraints.gridx = 2;
	panel.add(stopBrowse, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
		
	// eighth row
	constraints.gridy = 7;
		
	doStem = new JCheckBox("Stem collection", true);
	doStem.addItemListener(this);
	doStem.setToolTipText("<html>Select to enable stemming (conflation<br>" +
			      "of morphological variants) for this index</html>");
		
	stemmers = new JComboBox(sTypes);
	stemmers.setToolTipText("Select stemming algorithm.");
	stemmers.addActionListener(this);
		
	constraints.gridx = 0;
	panel.add(doStem, constraints);
		
	constraints.gridx = 1;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(stemmers, constraints);
	constraints.anchor = GridBagConstraints.LINE_END;
		
	memoryLim = new JComboBox(lims);
	memoryLim.setToolTipText("<html>How much memory to use while indexing.<br>" +
				 "A rule of thumb is no more than 3/4 of<br>" +
				 "your physical memory</html>");
	memoryLim.setSelectedIndex(4); // 512 MB
	label = new JLabel("Memory limit: ", JLabel.TRAILING);
	constraints.gridx = 2;
	constraints.anchor = GridBagConstraints.LINE_START;
	panel.add(label, constraints);
	constraints.gridx = 3;
	constraints.anchor = GridBagConstraints.LINE_END;
	panel.add(memoryLim, constraints);
		
	//put it all on a single tab, reorg and align nicely.
	//Adjust constraints for the content pane.
		
	tabbedPane.addTab("Index", icon, panel, "Index Options");
  
  // begin index field / metadata tab
  indexFieldPanel=makePanel();
  indexFieldPanel.setLayout(new java.awt.GridBagLayout());

  java.awt.GridBagConstraints gbc=new java.awt.GridBagConstraints();

  // fields table
  JPanel fieldsPanel=makePanel();

  fieldsPanel.setLayout(new java.awt.BorderLayout());
  fieldTableModel=new FieldTableModel();
  fieldTable=new JTable(fieldTableModel) {
  protected javax.swing.table.JTableHeader createDefaultTableHeader() {
      return new javax.swing.table.JTableHeader(columnModel) {
        public String getToolTipText(java.awt.event.MouseEvent e) {
          String tip=null;
          java.awt.Point p=e.getPoint();
          int index=columnModel.getColumnIndexAtX(p.x);
          int realIndex=columnModel.getColumn(index).getModelIndex();
          return fieldColumnTooltips[realIndex];
        }
      };
    }
  };
  
  fieldTable.getModel().addTableModelListener(this);
  fieldTable.getColumnModel().setColumnSelectionAllowed(false);
  fieldTable.setPreferredScrollableViewportSize(new Dimension(fieldTable.getPreferredScrollableViewportSize().width, 100));
  JScrollPane fieldTableScroll=new JScrollPane(fieldTable);
  fieldsPanel.add(fieldTableScroll, java.awt.BorderLayout.CENTER);

  JPanel fieldButtonPanel=new JPanel();
  btnAddField=new JButton("Add Field");
  btnAddField.setToolTipText("Adds a new (blank) field for indexing");
  btnAddField.addActionListener(this);
  fieldButtonPanel.add(btnAddField);
  btnRemoveField=new JButton("Remove Field");
  btnRemoveField.setToolTipText("Removes the currently selected field item");
  btnRemoveField.addActionListener(this);
  fieldButtonPanel.add(btnRemoveField);
  fieldsPanel.add(fieldButtonPanel, java.awt.BorderLayout.SOUTH);

  gbc.fill=GridBagConstraints.BOTH;
  gbc.gridx=0; gbc.gridy=0;
  indexFieldPanel.add(fieldsPanel, gbc);

  // offset annotation files table
  JPanel offsetAnnotationFilePanel=makePanel();
  offsetAnnotationFilePanel.setLayout(new java.awt.BorderLayout());
  offsetAnnotationFilesTableModel=new OffsetAnnotationTableModel();
  offsetAnnotationFileTable=new JTable(offsetAnnotationFilesTableModel) {
  protected javax.swing.table.JTableHeader createDefaultTableHeader() {
      return new javax.swing.table.JTableHeader(columnModel) {
        public String getToolTipText(java.awt.event.MouseEvent e) {
          String tip=null;
          java.awt.Point p=e.getPoint();
          int index=columnModel.getColumnIndexAtX(p.x);
          int realIndex=columnModel.getColumn(index).getModelIndex();
          return annotationsColumnTooltips[realIndex];
        }
      };
    }
  };
  offsetAnnotationFileTable.getModel().addTableModelListener(this);
  javax.swing.table.TableColumn offsetFileColumn=offsetAnnotationFileTable.getColumnModel().getColumn(1);
  offsetFileColumn.setCellEditor(new OffsetAnnotationFileCellEditor());
  offsetAnnotationFileTable.getColumnModel().setColumnSelectionAllowed(false);
  offsetAnnotationFileTable.setPreferredScrollableViewportSize(new Dimension(offsetAnnotationFileTable.getPreferredScrollableViewportSize().width, 100));
  int columnPrefWidth=(offsetAnnotationFileTable.getPreferredScrollableViewportSize().width / 2);
  for (int i=0; i < offsetAnnotationFileTable.getColumnCount(); i++) {
    javax.swing.table.TableColumn thisColumn=offsetAnnotationFileTable.getColumnModel().getColumn(i);
    thisColumn.setPreferredWidth(columnPrefWidth);
    thisColumn.setWidth(columnPrefWidth);
  }
  offsetAnnotationFileTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
  
  JScrollPane offsetFileTableScroll=new JScrollPane(offsetAnnotationFileTable);
  offsetAnnotationFilePanel.add(offsetFileTableScroll, java.awt.BorderLayout.CENTER);

  gbc.fill=GridBagConstraints.BOTH;
  gbc.gridx=0; gbc.gridy=1;

  indexFieldPanel.add(offsetAnnotationFilePanel, gbc);
  
  JPanel pnlHarvestLinks=makePanel();
  pnlHarvestLinks.setLayout(new java.awt.BorderLayout());
  JLabel lblHarvestLinks=new JLabel("Path to Anchor Text:");
  pnlHarvestLinks.add(lblHarvestLinks, java.awt.BorderLayout.WEST);
  txtPathToHarvestLinks=new JTextField();
  txtPathToHarvestLinks.setToolTipText("<html><i>(optional)</i> Path to the sorted output<br>" +
          "from the HarvestLinks program to<br>" +
          "include anchor text links (trecweb only)<br>" + 
          "<i>(leave blank for none)</i></html>"
          );
  pnlHarvestLinks.add(txtPathToHarvestLinks, java.awt.BorderLayout.CENTER);
  btnHarvestLinks=new JButton("Browse");
  btnHarvestLinks.addActionListener(this);
  pnlHarvestLinks.add(btnHarvestLinks, java.awt.BorderLayout.EAST);
  gbc.fill=GridBagConstraints.BOTH;
  gbc.gridx=0; gbc.gridy=2;
  indexFieldPanel.add(pnlHarvestLinks, gbc);

  tabbedPane.addTab("Fields", indexFieldPanel);  
		
	JPanel panel4 = makePanel();
	messages = new JTextArea(10,40);
	messages.setEditable(false);
		
	JScrollPane messageScrollPane = new JScrollPane(messages);
	panel4.add(messageScrollPane);
	tabbedPane.addTab("Status", icon, panel4, "Status Messages");
		
	JPanel buttons = new JPanel();
	go = new JButton("Build Index");
	go.setEnabled(false);
	go.addActionListener(this);	
	stop = new JButton("Quit");
	stop.addActionListener(this);
	buttons.add(go);
	buttons.add(stop);
	status = new JLabel("Ready...", indriIcon, JLabel.LEFT);
	add(tabbedPane, BorderLayout.NORTH);
	add(buttons, BorderLayout.CENTER);
	add(status, BorderLayout.SOUTH);
    }
    // gui helper functions.
    /** Create the applications menu bar.
	@return the JMenuBar.
    */    
    private JMenuBar makeMenuBar() {
	// menu
	menuBar = new JMenuBar();
	fileMenu = new JMenu("File");
	helpMenu = new JMenu("Help");
	menuBar.add(fileMenu);
	menuBar.add(Box.createHorizontalGlue());
	menuBar.add(helpMenu);
	fQuit = makeMenuItem("Quit");
	fileMenu.add(fQuit);
	hHelp = makeMenuItem("Help");
	helpMenu.add(hHelp);
	hAbout = makeMenuItem("About");
	helpMenu.add(hAbout);
	return menuBar;
    }
    /** Creates a JMenuItem with this as its ActionListener.
	@param label the label for the item.
	@return the created menu item.
    */    
    private JMenuItem makeMenuItem(String label) {
	JMenuItem item 	= new JMenuItem(label);
	item.addActionListener(this);
	return item;
    }
	
    /** Create a JPanel with BorderLayout.
	@return the Jpanel.
    */    
    private JPanel makePanel() {
	JPanel panel = new JPanel(new BorderLayout());
	return panel;
    }
	
    /** Returns an ImageIcon, or null if the path was invalid. 
	@param path the image file to load.
	@return an ImageIcon, or null if the path was invalid. 
    */
    protected static ImageIcon createImageIcon(String path) {
	java.net.URL imgURL = IndexUI.class.getResource(path);
	if (imgURL != null) {
	    return new ImageIcon(imgURL);
	} else {
	    return null;
	}
    }
	
    //Listeners
    /** Omnibus for responding to user actions. */
    public void actionPerformed(ActionEvent e) {
	Object source = e.getSource();
	if (source == browse) 	{
	    fc.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
	    int returnVal = fc.showOpenDialog(this);
	    if (returnVal == JFileChooser.APPROVE_OPTION) {
		File file = fc.getSelectedFile();
		iname.setText(file.getAbsolutePath());
	    }
	    fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
	} else if (source == cfbrowse) 	{
	    // pick file or directory
	    fc.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
	    // pick multiple at the same time
	    fc.setMultiSelectionEnabled(true);
	    // final because the inner class uses it.
	    final String regexp = filterString.getText();
	    if (regexp.length() > 0) {
		final String filtString = encodeRegexp(regexp);
		javax.swing.filechooser.FileFilter filt = 
		    new javax.swing.filechooser.FileFilter() {
			public boolean accept(File f) {
			    if (f.isDirectory()) {
				return true;
			    }
			    String name = f.getName();
			    return name.matches(filtString);
			}
			public String getDescription() {
			    return regexp + " files";
			}
		    };
		fc.addChoosableFileFilter(filt);
	    }
			
	    int returnVal = fc.showOpenDialog(this);
	    if (returnVal == JFileChooser.APPROVE_OPTION) {
		File [] files = fc.getSelectedFiles();
		for (int i = 0; i < files.length; i++) {
		    File file = files[i];
		    String docpath = file.getAbsolutePath();
		    // if user double clicked a directory to select,
		    // we get the directory name as the selected file
		    // in the intended directory.
		    // so check that the file exists and is a directory.
		    // if not, try the parent directory.
		    if (! file.exists())
          docpath = file.getParentFile().getAbsolutePath();
          cfModel.addElement(docpath);
          // add it to the offset annotation pathnames
          offsetAnnotationFilesTableModel.addFilename(docpath);
    		}
	    }
	    
	    fc.setMultiSelectionEnabled(false);
	    fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
	    // remove the filter.
	    fc.resetChoosableFileFilters();
	} else if (source == stopBrowse) {
	    int returnVal = fc.showOpenDialog(this);
	    if (returnVal == JFileChooser.APPROVE_OPTION) {
		File file = fc.getSelectedFile();
		stopwordlist.setText(file.getAbsolutePath());
		stopwords = stopwordlist.getText();
	    }
	} else if (source == stemmers) 	{
	} else if (source == docFormat) {
      if (docFormat.getSelectedItem().equals("trecweb")) {
         txtPathToHarvestLinks.setEnabled(true);
      } else {
        txtPathToHarvestLinks.setEnabled(false);
      }
	} else if (source == go) 	{
	    // show parameters and build index.
	    // sanity check first.
	    if (! safeToBuildIndex()) {
		// need more completed messages.
		status.setText("Unable to build " + iname.getText());
		return;
	    }
	    // flip to status tab -- change to 2 if only 3 tabs
	    tabbedPane.setSelectedIndex(2);
	    // start a new thread to run in so messages will be updated.
	    Runnable r = new Runnable() {
		    public void run() {
			buildIndex();
			ensureMessagesVisible();
		    }
		};
	    Thread t = new Thread(r);
	    t.start();
	} else if (source == stop) 	{
	    System.exit(0);
	} else if (source == fQuit) 	{
	    System.exit(0);
	} else if (source == cfRemove) 	{
	    Object [] selected = collectionFiles.getSelectedValues();
	    for (int i = 0; i < selected.length; i++) {
		cfModel.removeElement(selected[i]);
	    }
	} else if (source == hHelp) 	{
	    // pop up a help dialog
	    if (! helpFrame.isShowing()) {
		helpFrame.setLocationRelativeTo(tabbedPane);
		helpFrame.setVisible(true);
		helpFrame.toFront();
	    }
	} else if (source == hAbout) 	{
	    JOptionPane.showMessageDialog(this, aboutText, "About", 
					  JOptionPane.INFORMATION_MESSAGE,
					  createImageIcon(iconFile));
	} else if (source == btnAddField)  {
    // add a field item
    fieldTableModel.addNewField();
  } else if (source==btnRemoveField)  {
    // remove a field item
    int selectedRow=fieldTable.getSelectedRow();
    if (selectedRow > -1) {
      fieldTableModel.removeField(selectedRow);
    } else if (source == btnHarvestLinks)  {
       if (docFormat.getSelectedItem().equals("trecweb")) {
        JFileChooser fileChooser=new JFileChooser();
        fileChooser.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);
        fileChooser.setMultiSelectionEnabled(false);
        int retVal=fileChooser.showOpenDialog(this);
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
          txtPathToHarvestLinks.setText(fullPath);
        } // end if (retVal==JFileChooser.APPROVE_OPTION)
      }
    }
    
  }
  
	// at least one datafile and a name entered.
	boolean enabled = (cfModel.getSize() > 0 && 
			   iname.getText().length() > 0);
	go.setEnabled(enabled);
    }
    
    /** Listens for table model changes */
	public void tableChanged(TableModelEvent e) {
		if (e.getSource()==offsetAnnotationFilesTableModel) 
		{
			// offset annotations table changes
		}
		else if (e.getSource()==fieldTableModel) 
		{
			// fields table
		}
	}
    
	
    /** Listens to the check boxes. */
    public void itemStateChanged(ItemEvent e) {
	Object source = e.getItemSelectable();
	boolean change = (e.getStateChange() == ItemEvent.SELECTED);
	if (source == doStem) stemmers.setEnabled(change);
    }
	
    /** Listens to the index name text field. This enables updating the
	state of the BuildIndex button when the user types in the name
	of the index directly.
    */
    public void caretUpdate(CaretEvent event) {
	boolean enabled = (cfModel.getSize() > 0 && 
			   iname.getText().length() > 0);
	go.setEnabled(enabled);
    }
	
    /** Create the frame that shows the help file and render the html.
     */
    private void makeHelp() {
	java.net.URL helpURL = IndexUI.class.getResource(helpFile);
	JTextPane help = new JTextPane();
		
	//Create and set up the window.
	helpFrame = new JFrame("Indri Index Builder Help");
	help.setPreferredSize(new Dimension(650, 400));
	help.setEditable(false);
	help.addHyperlinkListener(new DocLinkListener(indriIcon.getImage()));
	JScrollPane scroller = new JScrollPane(help); 
	try {
	    help.setPage(helpURL);
	} catch (IOException ex) {
	    help.setText("Help file unavailable.");
	}

	helpFrame.getContentPane().add(scroller, BorderLayout.CENTER);
	helpFrame.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
	helpFrame.setIconImage(indriIcon.getImage());
	helpFrame.pack();
    }
	
	
    /**
     * Create the GUI and show it.  For thread safety,
     * this method should be invoked from the
     * event-dispatching thread.
     */
    private static void createAndShowGUI() {
	//Make sure we have nice window decorations.
	JFrame.setDefaultLookAndFeelDecorated(true);
	// For system look and feel
	try {
	    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
	} catch (Exception e) { 
	}
	//Create and set up the window.
	JFrame frame = new JFrame("Indri Index Builder");
	frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	frame.setIconImage(createImageIcon(iconFile).getImage());
	//Create and set up the content pane.
	IndexUI newContentPane = new IndexUI();
	newContentPane.setOpaque(true); //content panes must be opaque
	frame.getContentPane().add(newContentPane, BorderLayout.CENTER);
	frame.setJMenuBar(newContentPane.makeMenuBar());
	//Display the window.
	frame.pack();
	frame.setVisible(true);
    }
	
    // Utilities    
    /** Rewrite a shell filename pattern to a regular expression. <br>
     *  * -&gt; .*<br>
     ? -&gt; .?<br>
     Add ^ to beginning<br>
     Add $ to end<br>
     . -&gt; \.<br>
     @param regexp the filename pattern, eg "*.dat"
     @return a regular expression suitable for use with String.matches(), 
     eg "^.*\.dat$"
    */
    private String encodeRegexp(String regexp) {
	// rewrite shell fname pattern to regexp.
	// * -> .*
	// ? -> .?
	// Add ^,$
	// . -> \.
	String retval = "^" + regexp + "$";
	retval = retval.replaceAll("\\.", "\\.");
	retval = retval.replaceAll("\\*", ".*");
	retval = retval.replaceAll("\\?", ".?");
	return retval;
    }
	
    /** Tests for likely failure scenarios. If the path to the index 
	doesn't exist, fail. If the named index already exists, offer
	a choice do either overwrite, append, or abandon. If overwrite is 
	selected, removes all files in the target directory.
	This is potentially dangerous.
	@return true if it is safe to build the index otherwise false.
    */
    private boolean safeToBuildIndex() {
	appendIndex = false;
	// if iname is not an absolute path, rewrite it to be so.
	File idx = new File(iname.getText());
	String idxPath = idx.getAbsolutePath();
	idx = new File(idxPath);
	// if parent directory does not exist (typein error)
	// fail
	File dir = idx.getParentFile();
	if (!dir.exists()) {
	    messages.append("Unable to build " + idxPath +
			    "Directory " + dir.getAbsolutePath() +
			    " does not exist.\n");
	    return false;
	}
	// if idx exists, either fail or blow it away.
	// have to look inside for the manifest, etc.
	File manifest = new File(idxPath, "manifest");
	if (manifest.exists()) {
	    // need an append option (and attribute to hold it for open
	    // versus create call.
	    int val =
		JOptionPane.showConfirmDialog(this,
					      manifest.getAbsolutePath() + " exists. Choose YES to overwrite, NO to append to this index, CANCEL to do nothing",
					      "Overwrite or Append existing index",
					      JOptionPane.YES_NO_CANCEL_OPTION,
					      JOptionPane.WARNING_MESSAGE);
	    if (val == JOptionPane.CANCEL_OPTION) {
		// don't overwrite or append
		messages.append("Not building index " + idxPath + "\n");
		return false;
	    } else if (val == JOptionPane.NO_OPTION) {
		// don't overwrite. append
		messages.append("Appending new files to index " + idxPath + 
				"\n");
		appendIndex = true;
		return true;
	    } else if (val == JOptionPane.YES_OPTION) {
		// overwrite -- delete all index files.
		messages.append("Overwriting index " + idxPath + "\n");
		deleteDirectory(idx);
		return true;
	    } else { // any other option
		messages.append("Not building index " + idxPath + "\n");
		return false;
	    }
	}
	return true;
    }
    /** remove a directory and all of its files and subdirectories */
    private void deleteDirectory(File dir) {
	File [] files = dir.listFiles();
	for (int i = 0; i < files.length; i++) {
	    File f = files[i];
	    if (f.isDirectory())
		deleteDirectory(f);
	    messages.append("Deleting: " + f.getAbsolutePath() + "\n");
	    f.delete();
	}
    }
	
    /** Keeps the message pane showing the last line appended. */
    private void ensureMessagesVisible() {
	int len = messages.getText().length();
	try {
	    messages.scrollRectToVisible(messages.modelToView(len));
	} catch (javax.swing.text.BadLocationException ex) {
	    // don't care.
	}
    }
	
    /** Ask the IndexEnvironment to add the files.*/
    private void buildIndex() {
	Cursor wait = Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR);
	Cursor def = Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR);
	setCursor(wait);
	messages.setCursor(wait);
  
  int totalDocumentsIndexed=0;
  
	// if iname is not an absolute path, rewrite it to be so.
	File idx = new File(iname.getText());
	iname.setText(idx.getAbsolutePath());
	messages.append("Building " + iname.getText() + "...\n"); 
	status.setText("Building " + iname.getText() + "..."); 
	Thread bl = blinker(status.getText(), 
			    "Finished building " + iname.getText());
	// construct IndexEnvironment
	// set parameters
	// go.
	IndexEnvironment env = new IndexEnvironment();
	IndexStatus stat = new UIIndexStatus();
  
        try {
          // memory
          env.setMemory(encodeMem());

          // set the field definitions from the table
          java.util.Vector fieldVec=new java.util.Vector();
          java.util.Vector numericFields=new java.util.Vector();
          int numFields=fieldTable.getModel().getRowCount();
          for (int f=0; f < numFields; f++) {
                  String thisFieldName=((String)fieldTable.getModel().getValueAt(f, 0)).trim();
                  Boolean thisFieldNumeric=(Boolean)fieldTable.getModel().getValueAt(f, 1);
                  if ((thisFieldName.length() > 0) && (!fieldVec.contains(thisFieldName))) {
                          fieldVec.add(thisFieldName);
                          if (thisFieldNumeric.booleanValue()) {
                                  numericFields.add(thisFieldName);
                          }
                  }
          }

          String[] fields=new String[fieldVec.size()];
          for (int f=0; f < fieldVec.size(); f++) {
                  fields[f]=(String)fieldVec.get(f);
          }
          env.setIndexedFields(fields);

          // now, if there's any numeric fields, we need to set those...
          for (int f=0; f < numericFields.size(); f++) {
                  String thisNumericField=(String)numericFields.get(f);
                  env.setNumericField(thisNumericField, true, "NumericFieldAnnotator");
          }

          String [] metafields = colFields.getText().split(",");;
          String [] stopwords = new String[0];

                // this needs to address the forward/backward/metadata distinction.
          env.setMetadataIndexedFields(metafields, metafields);	
          String stops = stopwordlist.getText();
          if (! stops.equals("")) {
              // load the stopwords into an array.
              Vector tmp = new Vector();
              try {
            BufferedReader buf = new BufferedReader(new FileReader(stops));
            String line;
            while((line = buf.readLine()) != null) {
                tmp.add(line.trim());
            }
            buf.close();
              } catch (IOException ex) {
            // unlikely.
            showException(ex);
              }
              stopwords = (String[]) tmp.toArray(stopwords);
              env.setStopwords(stopwords);
          }

          if (doStem.isSelected()) {
              String stemmer = (String)stemmers.getSelectedItem();
              env.setStemmer(stemmer);
          }
          // add an empty string option
          String fileClass = (String)docFormat.getSelectedItem();
          // augment the environment as required
          Specification spec = env.getFileClassSpec(fileClass);
          java.util.Vector vec = new java.util.Vector();
          java.util.Vector incs = null;
          if (spec.include.length > 0)
              incs = new java.util.Vector();

          // indexed fields
          for (int i = 0; i < spec.index.length; i++)
              vec.add(spec.index[i]);
          for (int i = 0; i < fields.length; i++) {
              if (vec.indexOf(fields[i]) == -1)
            vec.add(fields[i]);
              // add to include tags only if there were some already.
              if (incs != null && incs.indexOf(fields[i]) == -1)
            incs.add(fields[i]);
          }

          if (vec.size() > spec.index.length) {
              // we added something.
              spec.index = new String[vec.size()];
              vec.copyInto(spec.index);
          }
          /* FIXME: forward/backward and plain metadata have to address the
                   issue of inserting entries for all names that conflate to a given
                   name.
                 */
          // metadata fields.
          vec.clear();
          for (int i = 0; i < spec.metadata.length; i++)
              vec.add(spec.metadata[i]);
          for (int i = 0; i < metafields.length; i++) {	    
              if (vec.indexOf(metafields[i]) == -1)
            vec.add(metafields[i]);
              // add to include tags only if there were some already.
              if (incs != null && incs.indexOf(metafields[i]) == -1)
            incs.add(metafields[i]);
          }

          if (vec.size() > spec.metadata.length) {
              // we added something.
              spec.metadata = new String[vec.size()];
              vec.copyInto(spec.metadata);
          }
          // update include if needed.
          if (incs != null && incs.size() > spec.include.length) {
              spec.include = new String[incs.size()];
              incs.copyInto(spec.include);
          }
          // update the environment.
          env.addFileClass(spec);
          
          if (fileClass.equals("trecweb")) {
            String pathHarvestLinks=txtPathToHarvestLinks.getText().trim();
            if (pathHarvestLinks.length() > 0) {
              env.setAnchorTextPath(pathHarvestLinks);
            }
          }

          String [] datafiles = formatDataFiles();
          String [] dummyStringArray=new String[0];
          String [] offsetFiles=(String[])dataFilesOffsetFiles.toArray(dummyStringArray);

          // create a new empty index (after parameters have been set).
          if (appendIndex)
              env.open(iname.getText(), stat);
          else 
              env.create(iname.getText(), stat);

          // don't let 'em quit easy while it is running.
          fQuit.setEnabled(false);
          stop.setEnabled(false);
          // do the building.
          for (int i = 0; i < datafiles.length; i++){
              String fname = datafiles[i];
              // if the fileClass is null, use 
              // env.addFile(fname);

              // is there an offsetAnnotation file for this?
              if ((offsetFiles.length > i) && (offsetFiles[i].length() > 0)) {
                  env.setOffsetAnnotationsPath(offsetFiles[i]);
              }

              env.addFile(fname, fileClass);
              totalDocumentsIndexed=env.documentsIndexed();
              ensureMessagesVisible();
          }
          env.close();
        } catch (Exception e) {
            // a lemur exception was tossed.
	messages.append("ERROR building " + iname.getText() + "\n" + e + "\n");
        }
        
        
	// now they can quit.
	fQuit.setEnabled(true);
	stop.setEnabled(true);
	blinking = false;
	bl.interrupt();
	setCursor(def);
	messages.setCursor(def);
	status.setText("Finished building " + iname.getText());
	messages.append("Finished building " + iname.getText() + "\n");
  messages.append("Total documents indexed: " + totalDocumentsIndexed + "\n\n");
	ensureMessagesVisible();
    }
	
	
    /** Create the datafiles list of strings.
	@return The list of files
    */
    private String[] formatDataFiles() {
	// handle directories, recursion, filename patterns	
	Vector accumulator = new Vector();
	String [] retval = new String[0];
		
  dataFilesOffsetFiles=new Vector();
  
	FileFilter filt = null;
	final String regexp = filterString.getText();
	if (regexp.length() > 0) {
	    final String filtString = encodeRegexp(regexp);
	    filt = new FileFilter() {
		    public boolean accept(File thisfile) {
			String name = thisfile.getName();
			return (thisfile.isDirectory() ||
				name.matches(filtString));
		    }
		};
	}
	Enumeration e = cfModel.elements();
  HashMap offsetFiles=offsetAnnotationFilesTableModel.getAllValues();
  
	while (e.hasMoreElements()) {
	    String s = (String) e.nextElement();
	    File file = new File(s);
      
      String thisOffsetFile="";
      if (offsetFiles.containsKey(s)) {
        thisOffsetFile=(String)offsetFiles.get(s);
      }
	    formatDataFiles(file, filt, accumulator, thisOffsetFile);		
	}
	retval = (String[]) accumulator.toArray(retval);
	return retval;
    }
	
    /** Accumulate filenames for the input list.
	If the File is a directory, iterate over all of the files
	in that directory that satisfy the filter. If recurse into
	subdirectories is selected and the File is a directory, 
	invoke recursivly on on all directories within the directory.
	@param accum Vector to accumulate file names recusively.
	@param file a File (either a file or directory)
	@param f the filename filter to apply.
    */
	
    private void formatDataFiles(File file, FileFilter f, Vector accum, String offsetFile) {
      if (file.isDirectory()) {
          // handle directory
          File [] files = file.listFiles(f);
          for (int i = 0; i < files.length; i++) {
        if (files[i].isDirectory()) {
            if (doRecurse.isSelected()) {
              formatDataFiles(files[i], f, accum, offsetFile);
            }
        } else {
            accum.add(files[i].getAbsolutePath());
            if (dataFilesOffsetFiles!=null) {
              dataFilesOffsetFiles.add(offsetFile);					
            }
        }
          }
      } else {
          accum.add(file.getAbsolutePath());
          if (dataFilesOffsetFiles!=null) {
            dataFilesOffsetFiles.add(offsetFile);					
          }

      }
    }
	
    private long encodeMem() {
	String s = ((String)memoryLim.getSelectedItem()).trim();
	int space = s.indexOf(' ');
	s = s.substring(0, space) + "000000";
	long retval = 0;
	try {
	    retval = Long.parseLong(s);
	} catch (Exception e) {
	}
	return retval;
    }
	
    /** Format an exception message in the messages text area.
	@param e the exception
    */
    private void showException(Exception e) {
	messages.append("\nERROR:\n");
	StringWriter msg = new StringWriter();
	PrintWriter w = new PrintWriter(msg);
	e.printStackTrace(w);
	w.close();
	messages.append(msg.toString());
	ensureMessagesVisible();
    }
	
    /** Is the blinker running? */
    private volatile boolean blinking = false;
    /** Make the status line blink while working. */
    private Thread blinker(final String s1, final String s2) {
	Thread blink;
	blink = new Thread(new Runnable() {
		public void run() {
		    String onText = s1;
		    String doneText = s2;
		    String offText = "";
		    int count = 0;
		    try {
			while (blinking) {
			    Thread.sleep(500);
			    if (count%2 == 0) {
				status.setText(offText);
			    } else {
				status.setText(onText);
			    }
			    count++;
			}
		    } catch (InterruptedException ex) {
			status.setText(doneText);
		    }
		}
	    });
	blinking = true;
	blink.start();
	return blink;
    }
    /** Fire it up.*/    
    public static void main(String[] args) {
	//Schedule a job for the event-dispatching thread:
	//creating and showing this application's GUI.
	javax.swing.SwingUtilities.invokeLater(new Runnable() {
		public void run() {
		    createAndShowGUI();
		}
	    });
    }
    class UIIndexStatus extends IndexStatus {
	public void status(int code, String documentFile, String error, 
			   int documentsIndexed, int documentsSeen) {
	    if (code == action_code.FileOpen.swigValue()) {
		messages.append("Documents: " + documentsIndexed + "\n");
		messages.append("Opened " + documentFile + "\n");
	    } else if (code == action_code.FileSkip.swigValue()) {
		messages.append("Skipped " + documentFile + "\n");
	    } else if (code == action_code.FileError.swigValue()) {
		messages.append("Error in " + documentFile + " : " + error + 
				"\n");
	    } else if (code == action_code.DocumentCount.swigValue()) {
		if( (documentsIndexed % 500) == 0)
		    messages.append( "Documents: " + documentsIndexed + "\n" );
	    }
	    int len = messages.getText().length();
	    try {
		messages.scrollRectToVisible(messages.modelToView(len));
	    } catch (javax.swing.text.BadLocationException ex) {
		// don't care.
	    }
	}
    }
}

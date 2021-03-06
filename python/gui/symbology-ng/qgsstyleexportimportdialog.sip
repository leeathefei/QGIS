class QgsStyleExportImportDialog : QDialog
{
%TypeHeaderCode
#include <qgsstyleexportimportdialog.h>
%End
  public:
    enum Mode
    {
      Export,
      Import
    };

    // constructor
    // mode argument must be 0 for saving and 1 for loading
    QgsStyleExportImportDialog( QgsStyle* style, QWidget *parent /TransferThis/ = NULL, Mode mode = Export );
    ~QgsStyleExportImportDialog();

    /**
     * @brief selectSymbols select symbols by name
     * @param symbolNames list of symbol names
     */
    void selectSymbols( const QStringList& symbolNames );
    /**
     * @brief deselectSymbols deselect symbols by name
     * @param symbolNames list of symbol names
     */
    void deselectSymbols( const QStringList& symbolNames );

  public slots:
    void doExportImport();
    /**
     * @brief selectByGroup open select by group dialog
     */
    void selectByGroup();
    /**
     * @brief selectAll selects all symbols
     */
    void selectAll();
    /**
     * @brief clearSelection deselects all symbols
     */
    void clearSelection();
    /**
     * Select the symbols belonging to the given tag
     * @param tagName the name of the tag to be selected
     */
    void selectTag( const QString& tagName );
    /**
     * Deselect the symbols belonging to the given tag
     * @param tagName the name of the tag to be deselected
     */
    void deselectTag( const QString& tagName );
    /**
     * @brief selectSmartgroup selects all symbols from a smart group
     * @param groupName
     */
    void selectSmartgroup( const QString& groupName );
    /**
     * @brief deselectSmartgroup deselects all symbols from a smart group
     * @param groupName
     */
    void deselectSmartgroup( const QString& groupName );

    void importTypeChanged( int );
    void browse();
};

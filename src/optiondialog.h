#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QDialog>
#include "setting.h"

// ファイル タブ
class FileTab : public QWidget
{
	Q_OBJECT
public:
	explicit FileTab(CSettings *pSetting, QWidget *parent = 0) ;

public slots:
	void slot_clickedCheckBox(bool) ;

private:
	CSettings		*m_pSetting ;
};

// アニメーションウィンドウ タブ
class AnimeWindowTab : public QWidget
{
	Q_OBJECT
public:
	explicit AnimeWindowTab(CSettings *pSetting, QWidget *parent = 0) ;

public slots:
	void slot_changeBGColor(QString) ;

private:
	CSettings		*m_pSetting ;
};

// イメージウィンドウ タブ
class ImageWindowTab : public QWidget
{
	Q_OBJECT
public:
	explicit ImageWindowTab(CSettings *pSetting, QWidget *parent = 0) ;

public slots:
	void slot_changeBGColor(QString) ;

private:
	CSettings		*m_pSetting ;
};

// オプションダイアログ
class OptionDialog : public QDialog
{
    Q_OBJECT
public:
	explicit OptionDialog(CSettings *pSetting, QWidget *parent = 0);

signals:

public slots:
};

#endif // OPTIONDIALOG_H
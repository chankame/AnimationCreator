#ifndef FRAMEDATASCALEFORM_H
#define FRAMEDATASCALEFORM_H

#include <QWidget>
#include "include.h"
#include "editdata.h"

namespace Ui {
	class FrameDataScaleForm;
}

class FrameDataScaleForm : public QWidget
{
	Q_OBJECT

public:
	explicit FrameDataScaleForm(CEditData *pEditData, QWidget *parent = 0);
	~FrameDataScaleForm();

public slots:
	void slot_clickedExec() ;

private:
    Ui::FrameDataScaleForm	*ui;
	CEditData				*m_pEditData ;
};

#endif // FRAMEDATASCALEFORM_H

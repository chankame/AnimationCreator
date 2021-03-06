#ifndef OBJECTSCALEFORM_H
#define OBJECTSCALEFORM_H

#include <QWidget>
#include "include.h"
#include "editdata.h"

namespace Ui {
    class ObjectScaleForm;
}

class ObjectScaleForm : public QWidget
{
    Q_OBJECT

public:
	explicit ObjectScaleForm(CEditData *pEditData, QWidget *parent = 0);
    ~ObjectScaleForm();

public slots:
	void slot_clickedExec() ;

private:
    Ui::ObjectScaleForm *ui;
	CEditData			*m_pEditData ;
};

#endif // OBJECTSCALEFORM_H

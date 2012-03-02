#include "command.h"
#include "animationform.h"
#include "debug.h"

/**
  アイテム追加 コマンド
  */
Command_AddItem::Command_AddItem(CEditData *pEditData, QString &str, QModelIndex &parent) :
	QUndoCommand(QObject::trUtf8("オブジェクト追加"))
{
	m_pEditData = pEditData ;
	m_str = str ;
	m_parentRow = m_pEditData->getObjectModel()->getRow(parent) ;
	m_row = -1 ;
}

void Command_AddItem::redo()
{
	QModelIndex index = m_pEditData->getObjectModel()->getIndex(m_parentRow) ;
	m_index = m_pEditData->getObjectModel()->addItem(m_str, index) ;
	m_row = m_pEditData->getObjectModel()->getRow(m_index) ;
}

void Command_AddItem::undo()
{
	if ( m_row < 0 ) { return ; }

//	m_pEditData->setSelIndex(QModelIndex());
	QModelIndex index = m_pEditData->getObjectModel()->getIndex(m_row) ;
	if ( index.isValid() ) {
		ObjectItem *pItem = m_pEditData->getObjectModel()->getItemFromIndex(index) ;
		m_str = pItem->getName() ;
		m_pEditData->getObjectModel()->removeItem(index) ;
	}
	m_row = -1 ;
}

/**
  アイテム削除 コマンド
  */
Command_DelItem::Command_DelItem(CEditData *pEditData, QModelIndex &index) :
	QUndoCommand(QObject::trUtf8("オブジェクト削除"))
{
	m_pEditData = pEditData ;
	m_relRow = index.row() ;
	m_row = m_pEditData->getObjectModel()->getRow(index) ;
	m_parentRow = m_pEditData->getObjectModel()->getRow(index.parent()) ;
	m_pItem = NULL ;

	qDebug() << "Command_DelItem relRow:" << m_relRow << " row:" << m_row << " parent row:" << m_parentRow ;
}

void Command_DelItem::redo()
{
	QModelIndex index = m_pEditData->getObjectModel()->getIndex(m_row) ;
	if ( index.isValid() ) {
		ObjectItem *pItem = m_pEditData->getObjectModel()->getItemFromIndex(index) ;
		if ( !m_pItem ) {
			m_pItem = new ObjectItem(pItem->getName(), pItem->parent()) ;
			m_pItem->copy(pItem) ;

			m_pEditData->getObjectModel()->removeItem(index) ;
			m_pEditData->getObjectModel()->updateIndex() ;
			qDebug() << "Command_DelItem redo exec" ;
		}
	}
}

void Command_DelItem::undo()
{
	if ( !m_pItem ) { return ; }

	QModelIndex index = m_pEditData->getObjectModel()->getIndex(m_parentRow) ;
	index = m_pEditData->getObjectModel()->insertItem(m_relRow, m_pItem->getName(), index) ;
	ObjectItem *p = m_pEditData->getObjectModel()->getItemFromIndex(index) ;
	if ( p ) {
		p->copy(m_pItem);
		m_pEditData->getObjectModel()->updateIndex() ;
		delete m_pItem ;
		m_pItem = NULL ;
		qDebug() << "Command_DelItem undo exec" ;
	}
}


/**
  フレームデータ追加コマンド
  */
Command_AddFrameData::Command_AddFrameData(CEditData		*pEditData,
										   QModelIndex		&index,
										   FrameData		&data,
										   QList<QWidget *>	&updateWidget) :
	QUndoCommand(QObject::trUtf8("オブジェクト追加"))
{
	qDebug() << "Command_AddFrameData" ;

	m_pEditData			= pEditData ;
	m_pObjModel			= pEditData->getObjectModel() ;
	m_row				= m_pObjModel->getRow(index) ;
	m_frameData			= data ;
	m_UpdateWidgetList	= updateWidget ;
	m_flag				= ObjectItem::kState_Disp ;
}

void Command_AddFrameData::redo()
{
	qDebug() << "Command_AddFrameData::redo()" ;

	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	qDebug() << "layer name:" << pItem->data(Qt::DisplayRole) ;

	FrameData *p = pItem->getFrameDataPtr(m_frameData.frame) ;
	if ( p ) {
		QMessageBox::warning(m_UpdateWidgetList[0], QObject::trUtf8("エラー 00"), QObject::trUtf8("不正なフレームデータが登録されました。直ちにプログラマに相談してください")) ;
		return ;
	}
	if ( m_frameData.frame > 9999 ) {
		QMessageBox::warning(m_UpdateWidgetList[0], QObject::trUtf8("エラー 01"), QObject::trUtf8("不正なフレームデータが登録されました。直ちにプログラマに相談してください")) ;
		return ;
	}

	pItem->addFrameData(m_frameData);
	pItem->setData(m_flag, Qt::CheckStateRole);

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_AddFrameData::undo()
{
	qDebug() << "Command_AddFrameData::undo()" ;
	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	qDebug() << "layer name:" << pItem->data(Qt::DisplayRole) ;
	m_flag = pItem->data(Qt::CheckStateRole).toInt() ;
	pItem->removeFrameData(m_frameData.frame) ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}



/**
  フレームデータ削除コマンド
  */
Command_DelFrameData::Command_DelFrameData(CEditData			*pEditData,
										   QModelIndex			&index,
										   int					frame,
										   QList<QWidget *>		&updateWidget) :
	QUndoCommand(QObject::trUtf8("フレームデータ削除"))
{
	qDebug() << "Command_DelFrameData" ;

	m_pEditData			= pEditData ;
	m_pObjModel			= pEditData->getObjectModel() ;
	m_row				= m_pObjModel->getRow(index) ;
	m_FrameData.frame	= frame ;
	m_UpdateWidgetList	= updateWidget ;
}

void Command_DelFrameData::redo()
{
	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	FrameData *p = pItem->getFrameDataPtr(m_FrameData.frame) ;
	if ( !p ) { return ; }

	m_FrameData = *p ;
	pItem->removeFrameData(m_FrameData.frame);

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_DelFrameData::undo()
{
	if ( m_FrameData.frame > 9999 ) {
		QMessageBox::warning(m_UpdateWidgetList[0], QObject::trUtf8("エラー 10"), QObject::trUtf8("不正なフレームデータが登録されました。直ちにプログラマに相談してください")) ;
		return ;
	}

	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( index.isValid() ) {
		ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
		pItem->addFrameData(m_FrameData) ;
	}

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}



/**
  フレームデータ編集コマンド
  */
Command_EditFrameData::Command_EditFrameData(CEditData			*pEditData,
											 QModelIndex		&index,
											 int				frame,
											 FrameData			&data,
											 FrameData			*pOld,
											 QList<QWidget *>	&updateWidget) :
	QUndoCommand(QObject::trUtf8("フレームデータ編集"))
{
	qDebug() << "Command_EditFrameData" ;

	m_pEditData = pEditData ;
	m_pObjModel = pEditData->getObjectModel() ;
	m_row		= m_pObjModel->getRow(index) ;
	m_frame		= frame ;
	m_FrameData = data ;
	m_UpdateWidgetList = updateWidget ;

	if ( pOld ) {
		m_OldFrameData = *pOld ;
		m_bSetOld = true ;
	}
}

void Command_EditFrameData::redo()
{
	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	FrameData *p = pItem->getFrameDataPtr(m_frame) ;
	if ( !p ) { return ; }

	if ( !m_bSetOld ) {
		m_OldFrameData = *p ;
	}
	m_bSetOld = false ;
	*p = m_FrameData ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		if ( m_UpdateWidgetList[i]->objectName() == "AnimationForm" ) {
			static_cast<AnimationForm *>(m_UpdateWidgetList[i])->slot_setUI(m_FrameData);
		}
		else {
			m_UpdateWidgetList[i]->update();
		}
	}
}

void Command_EditFrameData::undo()
{
	QModelIndex index = m_pObjModel->getIndex(m_row) ;
	if ( !index.isValid() ) { return ; }

	ObjectItem *pItem = m_pObjModel->getItemFromIndex(index) ;
	FrameData *p = pItem->getFrameDataPtr(m_frame) ;
	if ( !p ) { return ; }

	m_FrameData = *p ;
	*p = m_OldFrameData ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		if ( m_UpdateWidgetList[i]->objectName() == "AnimationForm" ) {
			static_cast<AnimationForm *>(m_UpdateWidgetList[i])->slot_setUI(m_OldFrameData);
		}
		else {
			m_UpdateWidgetList[i]->update();
		}
	}
}


/**
  オブジェクトコピーコマンド
  */
Command_CopyObject::Command_CopyObject( CEditData *pEditData, QModelIndex &index, QList<QWidget *> &updateWidget ) :
	QUndoCommand(QObject::trUtf8("オブジェクトコピー"))
{
	m_pEditData			= pEditData ;
	m_UpdateWidgetList	= updateWidget ;
	m_pObject			= NULL ;
	m_row				= -1 ;

	ObjectItem *p = pEditData->getObjectModel()->getObject(index) ;
	if ( p ) {
		m_pObject = new ObjectItem(p->getName() + "_copy", NULL) ;
		m_pObject->copy(p) ;
	}
}

void Command_CopyObject::redo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	if ( m_pObject ) {
		QModelIndex index = pModel->addItem(m_pObject->getName(), QModelIndex()) ;
		ObjectItem *pItem = pModel->getItemFromIndex(index) ;
		pItem->copy(m_pObject) ;
		pModel->updateIndex() ;
		m_row = pModel->getRow(index) ;
	}

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_CopyObject::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	if ( m_row >= 0 ) {
		QModelIndex index = pModel->getIndex(m_row) ;
		pModel->removeItem(index) ;
		m_row = -1 ;
	}
	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

/**
  レイヤコピー
  */
Command_CopyIndex::Command_CopyIndex( CEditData *pEditData, int row, ObjectItem *pLayer, QModelIndex parent, QList<QWidget *> &updateWidget ) :
	QUndoCommand(QObject::trUtf8("レイヤコピー"))
{
	m_pEditData			= pEditData ;
	m_relRow			= row ;
	m_parentRow			= m_pEditData->getObjectModel()->getRow(parent) ;
	m_UpdateWidgetList	= updateWidget ;

	m_pLayer = new ObjectItem(pLayer->getName() + QString("_copy"), NULL) ;
	m_pLayer->copy(pLayer) ;
}

void Command_CopyIndex::redo()
{
	if ( m_pLayer ) {
		CObjectModel *pModel = m_pEditData->getObjectModel() ;
		QModelIndex index ;

		index = pModel->getIndex(m_parentRow) ;
		if ( m_relRow < 0 ) {
			index = pModel->addItem(m_pLayer->getName(), index) ;
		}
		else {
			index = pModel->insertItem(m_relRow, m_pLayer->getName(), index) ;
		}
		if ( index.isValid() ) {
			ObjectItem *pItem = pModel->getItemFromIndex(index) ;
			pItem->copy(m_pLayer) ;
			pModel->updateIndex() ;
			m_row = pModel->getRow(index) ;
		}

//		m_pEditData->setSelIndex(index) ;
	}
	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_CopyIndex::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = pModel->getIndex(m_row) ;
	if ( index.isValid() ) {
		ObjectItem *pItem ;

		delete m_pLayer ;

		pItem = pModel->getItemFromIndex(index) ;
		m_pLayer = new ObjectItem(pItem->getName(), NULL) ;
		m_pLayer->copy(pItem) ;
		pModel->updateIndex() ;
		pModel->removeItem(index) ;
	}

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}


/**
  フレームデータ移動
  */
Command_MoveFrameData::Command_MoveFrameData(CEditData *pEditData, QModelIndex &index, int prevFrame, int nextFrame, QList<QWidget *> &updateWidget) :
	QUndoCommand(QObject::trUtf8("フレームデータ移動"))
{
	m_pEditData			= pEditData ;
	m_row				= m_pEditData->getObjectModel()->getRow(index) ;
	m_UpdateWidgetList	= updateWidget ;
	m_srcFrame			= prevFrame ;
	m_dstFrame			= nextFrame ;

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex i = pModel->getIndex(m_row) ;
	ObjectItem *pItem = pModel->getItemFromIndex(i) ;
	FrameData *pData ;
	pData = pItem->getFrameDataPtr(m_srcFrame) ;
	if ( pData ) { m_srcData = *pData ; }
	pData = pItem->getFrameDataPtr(m_dstFrame) ;
	if ( pData ) { m_dstData = *pData ; }
}

void Command_MoveFrameData::redo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = pModel->getIndex(m_row) ;
	ObjectItem *pItem = pModel->getItemFromIndex(index) ;
	if ( !pItem ) { return ; }

	// 移動先にコピー
	FrameData *pData = pItem->getFrameDataPtr(m_dstFrame) ;
	m_srcData.frame = m_dstFrame ;
	if ( pData ) { *pData = m_srcData ; }
	else {
		if ( m_srcData.frame > 9999 ) {
			QMessageBox::warning(m_UpdateWidgetList[0], QObject::trUtf8("エラー 20"), QObject::trUtf8("不正なフレームデータが登録されました。直ちにプログラマに相談してください")) ;
			return ;
		}
		pItem->addFrameData(m_srcData) ;
	}

	// 移動元を消す
	pItem->removeFrameData(m_srcFrame) ;

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_MoveFrameData::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = pModel->getIndex(m_row) ;
	ObjectItem *pItem = pModel->getItemFromIndex(index) ;
	if ( !pItem ) { return ; }

	// 移動元に戻す
	m_srcData.frame = m_srcFrame ;
	FrameData *pData = pItem->getFrameDataPtr(m_srcFrame) ;
	if ( pData ) {	// 移動元にデータがあるのはおかしい
		QMessageBox::warning(m_UpdateWidgetList[0], QObject::trUtf8("エラー 21"), QObject::trUtf8("不正なフレームデータが登録されました。直ちにプログラマに相談してください")) ;
		return ;
	}
	pItem->addFrameData(m_srcData) ;

	if ( m_dstData.frame != 0xffff ) {	// 元から移動先のデータがあった場合は戻す
		pData = pItem->getFrameDataPtr(m_dstFrame) ;
		if ( !pData ) {	// 移動先にデータがないのはおかしい
			QMessageBox::warning(m_UpdateWidgetList[0], QObject::trUtf8("エラー 22"), QObject::trUtf8("不正なフレームデータが登録されました。直ちにプログラマに相談してください")) ;
			return ;
		}
		*pData = m_dstData ;
	}
	else {	// ない場合は消す
		pItem->removeFrameData(m_dstFrame) ;
	}

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}


/**
  全フレームデータ移動
  */
Command_MoveAllFrameData::Command_MoveAllFrameData(CEditData *pEditData, QModelIndex &index, int prevFrame, int nextFrame, QList<QWidget *> &updateWidget) :
	QUndoCommand(QObject::trUtf8("全フレームデータ移動"))
{
	m_pEditData			= pEditData ;
	m_row				= m_pEditData->getObjectModel()->getRow(index) ;
	m_UpdateWidgetList	= updateWidget ;
	m_srcFrame			= prevFrame ;
	m_dstFrame			= nextFrame ;

	qDebug() << "moveAllFrameData" << "prev" << prevFrame << "next" << nextFrame ;
}

void Command_MoveAllFrameData::redo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = pModel->getIndex(m_row) ;
	ObjectItem *pItem = pModel->getObject(index) ;
	if ( !pItem ) { return ; }

	m_dstDatas.clear() ;
	save_frameData(pItem, m_srcFrame, m_dstFrame) ;

	if ( !pItem->validate() ) {
		QMessageBox::warning(m_UpdateWidgetList[0], QObject::trUtf8("エラー 30"), QObject::trUtf8("不正なフレームデータが登録されました。直ちにプログラマに相談してください")) ;
	}

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_MoveAllFrameData::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = pModel->getIndex(m_row) ;
	ObjectItem *pItem = pModel->getObject(index) ;
	if ( !pItem ) { return ; }

	restore_frameData(pItem, m_srcFrame, m_dstFrame) ;

	if ( !pItem->validate() ) {
		QMessageBox::warning(m_UpdateWidgetList[0], QObject::trUtf8("エラー 31"), QObject::trUtf8("不正なフレームデータが登録されました。直ちにプログラマに相談してください")) ;
	}

	for ( int i = 0 ; i < m_UpdateWidgetList.size() ; i ++ ) {
		m_UpdateWidgetList[i]->update();
	}
}

void Command_MoveAllFrameData::save_frameData(ObjectItem *pItem, int srcFrame, int dstFrame)
{
	int row = m_pEditData->getObjectModel()->getRow(pItem->getIndex()) ;

	FrameData *pSrc = pItem->getFrameDataPtr(srcFrame) ;
	FrameData *pDst = pItem->getFrameDataPtr(dstFrame) ;
	if ( pSrc ) {
		if ( pDst ) {
			m_dstDatas.append(qMakePair(row, *pDst)) ;
			pItem->removeFrameData(dstFrame) ;
		}
		pSrc->frame = dstFrame ;
		qDebug() << "save_frameData move" << pItem->getName() ;
	}

	for ( int i = 0 ; i < pItem->childCount() ; i ++ ) {
		save_frameData(pItem->child(i), srcFrame, dstFrame) ;
	}
}

void Command_MoveAllFrameData::restore_frameData(ObjectItem *pItem, int srcFrame, int dstFrame)
{
	int row = m_pEditData->getObjectModel()->getRow(pItem->getIndex()) ;

	FrameData *pSrc = pItem->getFrameDataPtr(srcFrame) ;
	FrameData *pDst = pItem->getFrameDataPtr(dstFrame) ;
	if ( pDst ) {
		if ( pSrc ) {
			QMessageBox::warning(m_UpdateWidgetList[0], QObject::trUtf8("エラー 32"), QObject::trUtf8("不正なフレームデータが登録されました。直ちにプログラマに相談してください")) ;
			return ;
		}
		pDst->frame = srcFrame ;

		for ( int i = 0 ; i < m_dstDatas.size() ; i ++ ) {
			if ( m_dstDatas.at(i).first != row ) { continue ; }
			pItem->addFrameData(m_dstDatas.at(i).second) ;
			break ;
		}
	}

	for ( int i = 0 ; i < pItem->childCount() ; i ++ ) {
		restore_frameData(pItem->child(i), srcFrame, dstFrame) ;
	}
}


/**
  ツリーアイテム上に移動
  */
Command_MoveItemUp::Command_MoveItemUp(CEditData *pEditData, const QModelIndex &index) :
	QUndoCommand(QObject::trUtf8("アイテム上移動"))
{
	m_pEditData	= pEditData ;
	m_index		= index ;

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	ObjectItem *pItem = pModel->getItemFromIndex(index) ;
	m_pItem = new ObjectItem(pItem->getName(), NULL) ;
	m_pItem->copy(pItem) ;
}

void Command_MoveItemUp::redo()
{
	QModelIndex oldIndex = m_index ;
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	pModel->removeItem(oldIndex) ;
	m_index = pModel->insertItem(oldIndex.row()-1, m_pItem->getName(), oldIndex.parent()) ;
	ObjectItem *pItem = pModel->getItemFromIndex(m_index) ;
	pItem->copy(m_pItem) ;
	pModel->updateIndex();
	m_pEditData->getTreeView()->setCurrentIndex(m_index) ;
}

void Command_MoveItemUp::undo()
{
	QModelIndex oldIndex = m_index ;
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	pModel->removeItem(oldIndex) ;
	m_index = pModel->insertItem(oldIndex.row()+1, m_pItem->getName(), oldIndex.parent()) ;
	ObjectItem *pItem = pModel->getItemFromIndex(m_index) ;
	pItem->copy(m_pItem) ;
	pModel->updateIndex();
	m_pEditData->getTreeView()->setCurrentIndex(m_index) ;
}


/**
  ツリーアイテム下に移動
  */
Command_MoveItemDown::Command_MoveItemDown(CEditData *pEditData, const QModelIndex &index) :
	QUndoCommand(QObject::trUtf8("アイテム下移動"))
{
	m_pEditData	= pEditData ;
	m_index		= index ;

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	ObjectItem *pItem = pModel->getItemFromIndex(index) ;
	m_pItem = new ObjectItem(pItem->getName(), NULL) ;
	m_pItem->copy(pItem) ;
}

void Command_MoveItemDown::redo()
{
	QModelIndex oldIndex = m_index ;
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	pModel->removeItem(oldIndex) ;
	m_index = pModel->insertItem(oldIndex.row()+1, m_pItem->getName(), oldIndex.parent()) ;
	ObjectItem *pItem = pModel->getItemFromIndex(m_index) ;
	pItem->copy(m_pItem) ;
	pModel->updateIndex();
	m_pEditData->getTreeView()->setCurrentIndex(m_index) ;
}

void Command_MoveItemDown::undo()
{
	QModelIndex oldIndex = m_index ;
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	pModel->removeItem(oldIndex) ;
	m_index = pModel->insertItem(oldIndex.row()-1, m_pItem->getName(), oldIndex.parent()) ;
	ObjectItem *pItem = pModel->getItemFromIndex(m_index) ;
	pItem->copy(m_pItem) ;
	pModel->updateIndex();
	m_pEditData->getTreeView()->setCurrentIndex(m_index) ;
}


/**
  UVスケール変更
  */
Command_ScaleUv::Command_ScaleUv(CEditData *pEditData, double scale) :
	QUndoCommand(QObject::trUtf8("UVスケール変更"))
{
	m_pEditData = pEditData ;
	m_scale = scale ;
}

void Command_ScaleUv::redo()
{
	m_changeFrameDatas.clear();

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	for ( int i = 0 ; i < pModel->rowCount(QModelIndex()) ; i ++ ) {
		QModelIndex index = pModel->index(i) ;
		ObjectItem *pObj = pModel->getObject(index) ;
		if ( !pObj ) { continue ; }
		save_framedata(pObj) ;
	}
}

void Command_ScaleUv::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	for ( int i = 0 ; i < m_changeFrameDatas.size() ; i ++ ) {
		const QPair<int, FrameData> d = m_changeFrameDatas.at(i) ;
		QModelIndex index = pModel->getIndex(d.first) ;
		ObjectItem *pItem = pModel->getItemFromIndex(index) ;
		if ( !pItem ) { continue ; }
		FrameData *pFrameData = pItem->getFrameDataPtr(d.second.frame) ;
		if ( !pFrameData ) {
			qDebug() << "Command_ScaleUv pFrameData==NULLpo" ;
			continue ;
		}
		*pFrameData = d.second ;
	}
}

void Command_ScaleUv::save_framedata(ObjectItem *pItem)
{
	int i ;
	int row = m_pEditData->getObjectModel()->getRow(pItem->getIndex()) ;

	int frameNum = pItem->getMaxFrameNum(false) ;
	for ( i = 0 ; i <= frameNum ; i ++ ) {
		FrameData *p = pItem->getFrameDataPtr(i) ;
		if ( !p ) { continue ; }
//		if ( p->nImage != m_imageNo ) { continue ; }

		QPair<int, FrameData> d = qMakePair(row, *p) ;
		m_changeFrameDatas.append(d) ;

		p->left *= m_scale ;
		p->right *= m_scale ;
		p->top *= m_scale ;
		p->bottom *= m_scale ;
		p->pos_x *= m_scale ;
		p->pos_y *= m_scale ;
		p->center_x *= m_scale ;
		p->center_y *= m_scale ;
//		p->fScaleX *= m_scale ;
//		p->fScaleY *= m_scale ;
	}

	for ( i = 0 ; i < pItem->childCount() ; i ++ ) {
		save_framedata(pItem->child(i)) ;
	}
}


/**
  フレームスケール変更
  */
Command_ScaleFrame::Command_ScaleFrame(CEditData *pEditData, double scale) :
	QUndoCommand(QObject::trUtf8("フレームスケール変更"))
{
	m_pEditData = pEditData ;
	m_scale = scale ;
}

void Command_ScaleFrame::redo()
{
	m_changeFrameDatas.clear();

	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	QModelIndex index = m_pEditData->getSelIndex() ;
	ObjectItem *pItem = pModel->getObject(index) ;
	if ( !pItem ) { return ; }

	save_framedata(pItem) ;
	if ( !pItem->validate() ) {
		if ( qApp->activeWindow() ) {
			QMessageBox::warning(qApp->activeWindow(), QObject::trUtf8("エラー 40"), QObject::trUtf8("不正なフレームデータが登録されました。直ちにプログラマに相談してください")) ;
		}
		else {
			qDebug() << "エラー 23:不正なフレームデータが登録されました。直ちにプログラマに相談してください" ;
		}
	}
}

void Command_ScaleFrame::undo()
{
	CObjectModel *pModel = m_pEditData->getObjectModel() ;
	for ( int i = 0 ; i < m_changeFrameDatas.size() ; i ++ ) {
		const QPair<int, QList<FrameData> > d = m_changeFrameDatas.at(i) ;
		QModelIndex index = pModel->getIndex(d.first) ;
		ObjectItem *pItem = pModel->getItemFromIndex(index) ;
		if ( !pItem ) { continue ; }
		pItem->setFrameDatas(d.second) ;
	}
}

void Command_ScaleFrame::save_framedata(ObjectItem *pItem)
{
	int i ;
	int row = m_pEditData->getObjectModel()->getRow(pItem->getIndex()) ;

	pItem->sortFrameData() ;
	const QList<FrameData> &olds = pItem->getFrameData() ;
	m_changeFrameDatas.append(qMakePair(row, olds)) ;

	QList<FrameData> news ;
	for ( i = 0 ; i < olds.size() ; i ++ ) {
		FrameData d = olds[i] ;
		d.frame *= m_scale ;
		for ( int j = 0 ; j < news.size() ; j ++ ) {
			if ( news.at(j).frame == d.frame ) {
				d.frame ++ ;
				j = -1 ;
			}
		}
		news.append(d) ;
	}
	pItem->setFrameDatas(news) ;

	for ( i = 0 ; i < pItem->childCount() ; i ++ ) {
		save_framedata(pItem->child(i)) ;
	}
}



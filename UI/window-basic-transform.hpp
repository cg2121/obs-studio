#pragma once

#include <obs.hpp>
#include <memory>

#include "ui_OBSBasicTransform.h"

class OBSBasic;
class QListWidgetItem;

class OBSBasicTransform : public QDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::OBSBasicTransform> ui;

	OBSBasic *main;
	OBSSceneItem item;
	OBSSignal channelChangedSignal;
	OBSSignal transformSignal;
	OBSSignal removeSignal;
	OBSSignal selectSignal;
	OBSSignal deselectSignal;

	std::string undo_data;

	bool ignoreTransformSignal = false;
	bool ignoreItemChange = false;

	void HookWidget(QWidget *widget, const char *signal, const char *slot);

	void SetScene(OBSScene scene);
	void SetItem(OBSSceneItem newItem);

	static void OBSChannelChanged(void *param, calldata_t *data);

	static void OBSSceneItemTransform(void *param, calldata_t *data);
	static void OBSSceneItemRemoved(void *param, calldata_t *data);
	static void OBSSceneItemSelect(void *param, calldata_t *data);
	static void OBSSceneItemDeselect(void *param, calldata_t *data);

	double sizeAspect = 1.0;
	double boundsAspect = 1.0;

private slots:
	void RefreshControls();
	void SetItemQt(OBSSceneItem newItem);
	void OnBoundsType(int index);
	void OnControlChanged();
	void OnCropChanged();
	void on_resetButton_clicked();

	void on_sizeX_valueChanged(double value);
	void on_sizeY_valueChanged(double value);
	void on_boundsWidth_valueChanged(double value);
	void on_boundsHeight_valueChanged(double value);

public:
	OBSBasicTransform(OBSBasic *parent);
	~OBSBasicTransform();

public slots:
	void OnSceneChanged(QListWidgetItem *current, QListWidgetItem *prev);
};

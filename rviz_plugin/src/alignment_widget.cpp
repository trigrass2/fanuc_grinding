
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QFuture>
#include <QtConcurrentRun>

#include "alignment_widget.h"

fanuc_grinding_rviz_plugin::AlignmentWidget::AlignmentWidget(QWidget* parent) : QWidget(parent)
{
  setObjectName("AlignmentWidget_");
  alignment_button_ = new QPushButton;
  alignment_button_->setText("Start Alignment");

  QVBoxLayout* alignment_layout = new QVBoxLayout(this);
  alignment_layout->addWidget(alignment_button_);

  // Connect handlers
  connect(alignment_button_, SIGNAL(released()), this, SLOT(AlignmentButtonHandler()));

  //Setup client
  alignment_service_ = nh_.serviceClient<fanuc_grinding_alignment::AlignmentService>("alignment_service");

  QFuture<void> future = QtConcurrent::run(this, &fanuc_grinding_rviz_plugin::AlignmentWidget::connectToServices);
}

void fanuc_grinding_rviz_plugin::AlignmentWidget::triggerSave()
{
  Q_EMIT GUIChanged();
  updateInternalValues();
  updateGUI();
}

fanuc_grinding_alignment::AlignmentService::Request fanuc_grinding_rviz_plugin::AlignmentWidget::getAlignmentParams()
{
  return alignment_params_;
}

void fanuc_grinding_rviz_plugin::AlignmentWidget::setAlignmentParams(fanuc_grinding_alignment::AlignmentService::Request params)
{
  updateGUI();
}

void fanuc_grinding_rviz_plugin::AlignmentWidget::updateGUI()
{
  // Not implemented yet
}

void fanuc_grinding_rviz_plugin::AlignmentWidget::updateInternalValues()
{
  // Not implemented yet
}

void fanuc_grinding_rviz_plugin::AlignmentWidget::setCADAndScanParams(const QString cad_filename,
                                                                const QString cad_marker_name,
                                                                const QString scan_filename,
                                                                const QString scan_marker_name)
{
  alignment_params_.CADFileName = cad_filename.toStdString();
  alignment_params_.CADMarkerName = cad_marker_name.toStdString();
  alignment_params_.ScanFileName = scan_filename.toStdString();
  alignment_params_.ScanMarkerName = scan_marker_name.toStdString();
}

void fanuc_grinding_rviz_plugin::AlignmentWidget::AlignmentButtonHandler()
{
  // get CAD and Scan params which are stored in grinding rviz plugin
  Q_EMIT getCADAndScanParams();
  // Fill in the request
  srv_alignment_.request = getAlignmentParams();
  //srv_.request.*request* = *value*;
  // Start client service call in an other thread
  QFuture<void> future = QtConcurrent::run(this, &AlignmentWidget::Alignment);
}

void fanuc_grinding_rviz_plugin::AlignmentWidget::Alignment()
{
  // Disable UI
  Q_EMIT enablePanel(false);

  // Call client service
  alignment_service_.call(srv_alignment_);
  Q_EMIT sendStatus(QString::fromStdString(srv_alignment_.response.ReturnMessage));

  if(srv_alignment_.response.ReturnStatus == true)
    Q_EMIT enablePanelComparison();
  else
  {
    Q_EMIT sendMsgBox("Error aligning meshes",
                      QString::fromStdString(srv_alignment_.response.ReturnMessage), "");
  }

  // Re-enable UI
  Q_EMIT enablePanel(true); // Enable UI
}

void fanuc_grinding_rviz_plugin::AlignmentWidget::connectToServices()
{
  Q_EMIT enablePanel(false);

  // Check offset_move_robot_ connection
  Q_EMIT sendStatus("Connecting to service");
  while (ros::ok())
  {
    if (alignment_service_.waitForExistence(ros::Duration(2)))
    {
      ROS_INFO_STREAM(objectName().toStdString() + " RViz panel connected to the service " << alignment_service_.getService());
      Q_EMIT sendStatus(QString::fromStdString("RViz panel connected to the service: " + alignment_service_.getService()));
      break;
    }
    else
    {
      ROS_WARN_STREAM(objectName().toStdString() + " RViz panel could not connect to ROS service:\n\t" << alignment_service_.getService());
      Q_EMIT sendStatus(QString::fromStdString("RViz panel could not connect to ROS service: " + alignment_service_.getService()));
      sleep(1);
    }
  }

  ROS_INFO_STREAM(objectName().toStdString() + " service connections have been made");
  Q_EMIT sendStatus("Ready to take commands");
  Q_EMIT enablePanel(true);
}

// Save all configuration data from this panel to the given Config object
void fanuc_grinding_rviz_plugin::AlignmentWidget::save(rviz::Config config)
{
  // NOT IMPLEMENTED YET
}

// Load all configuration data for this panel from the given Config object.
void fanuc_grinding_rviz_plugin::AlignmentWidget::load(const rviz::Config& config)
{
  // NOT IMPLEMENTED YET
}
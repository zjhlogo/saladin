
/*********************************************************************
 * \file   PathLineEditor.h
 * \brief  
 * 
 * \author zjhlogo
 * \date   02/25/2022

 * *********************************************************************/

#pragma once

class PathLineEditor : public QLineEdit
{
    Q_OBJECT

public:
    PathLineEditor(QWidget* parent = 0);
    ~PathLineEditor();

protected:
    virtual void focusInEvent(QFocusEvent* e) override;
    virtual void focusOutEvent(QFocusEvent* e) override;
};

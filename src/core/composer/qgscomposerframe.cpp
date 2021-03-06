/***************************************************************************
                              qgscomposerframe.cpp
    ------------------------------------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerframe.h"
#include "qgscomposermultiframe.h"
#include "qgscomposition.h"

QgsComposerFrame::QgsComposerFrame( QgsComposition *c, QgsComposerMultiFrame *mf, qreal x, qreal y, qreal width, qreal height )
  : QgsComposerItem( x, y, width, height, c )
  , mMultiFrame( mf )
  , mHidePageIfEmpty( false )
  , mHideBackgroundIfEmpty( false )
{

  //default to no background
  setBackgroundEnabled( false );

  //repaint frame when multiframe content changes
  connect( mf, &QgsComposerMultiFrame::contentsChanged, this, &QgsComposerItem::repaint );
  if ( mf )
  {
    //force recalculation of rect, so that multiframe specified sizes can be applied
    setSceneRect( QRectF( pos().x(), pos().y(), rect().width(), rect().height() ) );
  }
}

QgsComposerFrame::QgsComposerFrame()
  : QgsComposerItem( 0, 0, 0, 0, nullptr )
  , mMultiFrame( nullptr )
  , mHidePageIfEmpty( false )
  , mHideBackgroundIfEmpty( false )
{
  //default to no background
  setBackgroundEnabled( false );
}

bool QgsComposerFrame::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  QDomElement frameElem = doc.createElement( QStringLiteral( "ComposerFrame" ) );
  frameElem.setAttribute( QStringLiteral( "sectionX" ), QString::number( mSection.x() ) );
  frameElem.setAttribute( QStringLiteral( "sectionY" ), QString::number( mSection.y() ) );
  frameElem.setAttribute( QStringLiteral( "sectionWidth" ), QString::number( mSection.width() ) );
  frameElem.setAttribute( QStringLiteral( "sectionHeight" ), QString::number( mSection.height() ) );
  frameElem.setAttribute( QStringLiteral( "hidePageIfEmpty" ), mHidePageIfEmpty );
  frameElem.setAttribute( QStringLiteral( "hideBackgroundIfEmpty" ), mHideBackgroundIfEmpty );
  elem.appendChild( frameElem );

  return _writeXml( frameElem, doc );
}

bool QgsComposerFrame::readXml( const QDomElement &itemElem, const QDomDocument &doc )
{
  double x = itemElem.attribute( QStringLiteral( "sectionX" ) ).toDouble();
  double y = itemElem.attribute( QStringLiteral( "sectionY" ) ).toDouble();
  double width = itemElem.attribute( QStringLiteral( "sectionWidth" ) ).toDouble();
  double height = itemElem.attribute( QStringLiteral( "sectionHeight" ) ).toDouble();
  mSection = QRectF( x, y, width, height );
  mHidePageIfEmpty = itemElem.attribute( QStringLiteral( "hidePageIfEmpty" ), QStringLiteral( "0" ) ).toInt();
  mHideBackgroundIfEmpty = itemElem.attribute( QStringLiteral( "hideBackgroundIfEmpty" ), QStringLiteral( "0" ) ).toInt();
  QDomElement composerItem = itemElem.firstChildElement( QStringLiteral( "ComposerItem" ) );
  if ( composerItem.isNull() )
  {
    return false;
  }
  return _readXml( composerItem, doc );
}

void QgsComposerFrame::setHidePageIfEmpty( const bool hidePageIfEmpty )
{
  mHidePageIfEmpty = hidePageIfEmpty;
}

void QgsComposerFrame::setHideBackgroundIfEmpty( const bool hideBackgroundIfEmpty )
{
  if ( hideBackgroundIfEmpty == mHideBackgroundIfEmpty )
  {
    return;
  }

  mHideBackgroundIfEmpty = hideBackgroundIfEmpty;
  update();
}

bool QgsComposerFrame::isEmpty() const
{
  if ( !mMultiFrame )
  {
    return true;
  }

  double multiFrameHeight = mMultiFrame->totalSize().height();
  if ( multiFrameHeight <= mSection.top() )
  {
    //multiframe height is less than top of this frame's visible portion
    return true;
  }

  return false;

}

QgsExpressionContext QgsComposerFrame::createExpressionContext() const
{
  if ( !mMultiFrame )
    return QgsComposerItem::createExpressionContext();

  //start with multiframe's context
  QgsExpressionContext context = mMultiFrame->createExpressionContext();
  //add frame's individual context
  context.appendScope( QgsExpressionContextUtils::composerItemScope( this ) );

  return context;
}

QString QgsComposerFrame::displayName() const
{
  if ( !id().isEmpty() )
  {
    return id();
  }

  if ( mMultiFrame )
  {
    return mMultiFrame->displayName();
  }

  return tr( "<frame>" );
}

void QgsComposerFrame::setSceneRect( const QRectF &rectangle )
{
  QRectF fixedRect = rectangle;

  if ( mMultiFrame )
  {
    //calculate index of frame
    int frameIndex = mMultiFrame->frameIndex( this );

    QSizeF fixedSize = mMultiFrame->fixedFrameSize( frameIndex );
    if ( fixedSize.width() > 0 )
    {
      fixedRect.setWidth( fixedSize.width() );
    }
    if ( fixedSize.height() > 0 )
    {
      fixedRect.setHeight( fixedSize.height() );
    }

    //check minimum size
    QSizeF minSize = mMultiFrame->minFrameSize( frameIndex );
    fixedRect.setWidth( qMax( minSize.width(), fixedRect.width() ) );
    fixedRect.setHeight( qMax( minSize.height(), fixedRect.height() ) );
  }

  QgsComposerItem::setSceneRect( fixedRect );
}

void QgsComposerFrame::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  Q_UNUSED( itemStyle );
  Q_UNUSED( pWidget );

  if ( !painter )
  {
    return;
  }
  if ( !shouldDrawItem() )
  {
    return;
  }

  bool empty = isEmpty();

  if ( !empty || !mHideBackgroundIfEmpty )
  {
    drawBackground( painter );
  }
  if ( mMultiFrame )
  {
    //calculate index of frame
    int frameIndex = mMultiFrame->frameIndex( this );
    mMultiFrame->render( painter, mSection, frameIndex );
  }

  if ( !empty || !mHideBackgroundIfEmpty )
  {
    drawFrame( painter );
  }
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerFrame::beginItemCommand( const QString &text )
{
  if ( mComposition )
  {
    mComposition->beginMultiFrameCommand( multiFrame(), text );
  }
}

void QgsComposerFrame::endItemCommand()
{
  if ( mComposition )
  {
    mComposition->endMultiFrameCommand();
  }
}

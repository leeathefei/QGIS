/***************************************************************************
                      qgsfeature.h - Spatial Feature Class
                     --------------------------------------
Date                 : 09-Sep-2003
Copyright            : (C) 2003 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATURE_H
#define QGSFEATURE_H

#include "qgis_core.h"
#include <QMap>
#include <QString>
#include <QVariant>
#include <QList>
#include <QVector>
#include <QSet>
#include <QExplicitlySharedDataPointer>

#include "qgsfields.h"

class QgsGeometry;
class QgsRectangle;
class QgsFeature;
class QgsFeaturePrivate;

// feature id class (currently 64 bit)

// 64 bit feature ids
typedef qint64 QgsFeatureId;
#define FID_IS_NEW(fid)     (fid<0)
#define FID_TO_NUMBER(fid)  static_cast<qint64>(fid)
#define FID_TO_STRING(fid)  QString::number( fid )
#define STRING_TO_FID(str)  (str).toLongLong()

// key = field index, value = field value
typedef QMap<int, QVariant> QgsAttributeMap;

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

/** \ingroup core
 * A vector of attributes. Mostly equal to QVector<QVariant>.
 */
class CORE_EXPORT QgsAttributes : public QVector<QVariant>
{
  public:
    QgsAttributes()
      : QVector<QVariant>()
    {}

    /**
     * Create a new vector of attributes with the given size
     *
     * @param size Number of attributes
     */
    QgsAttributes( int size )
      : QVector<QVariant>( size )
    {}

    /**
     * Constructs a vector with an initial size of size elements. Each element is initialized with value.
     * @param size Number of elements
     * @param v    Initial value
     */
    QgsAttributes( int size, const QVariant &v )
      : QVector<QVariant>( size, v )
    {}

    /**
     * Copies another vector of attributes
     * @param v Attributes to copy
     */
    QgsAttributes( const QVector<QVariant> &v )
      : QVector<QVariant>( v )
    {}

    /**
     * @brief Compares two vectors of attributes.
     * They are considered equal if all their members contain the same value and NULL flag.
     * This was introduced because the default Qt implementation of QVariant comparison does not
     * handle NULL values for certain types (like int).
     *
     * @param v The attributes to compare
     * @return True if v is equal
     */
    bool operator==( const QgsAttributes &v ) const
    {
      if ( size() != v.size() )
        return false;
      const QVariant *b = constData();
      const QVariant *i = b + size();
      const QVariant *j = v.constData() + size();
      while ( i != b )
        if ( !( *--i == *--j && i->isNull() == j->isNull() ) )
          return false;
      return true;
    }

    /**
     * Returns a QgsAttributeMap of the attribute values. Null values are
     * excluded from the map.
     * @note added in QGIS 3.0
     * @note not available in Python bindings
     */
    QgsAttributeMap toMap() const;

    inline bool operator!=( const QgsAttributes &v ) const { return !( *this == v ); }
};

class QgsField;

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfeature.cpp.
 * See details in QEP #17
 ****************************************************************************/

/** \ingroup core
 * The feature class encapsulates a single feature including its id,
 * geometry and a list of field/values attributes.
 * \note QgsFeature objects are implicitly shared.
 * @author Gary E.Sherman
 */
class CORE_EXPORT QgsFeature
{
    Q_GADGET

    Q_PROPERTY( QgsFeatureId id READ id WRITE setId )
    Q_PROPERTY( QgsAttributes attributes READ attributes WRITE setAttributes )
    Q_PROPERTY( QgsFields fields READ fields WRITE setFields )

  public:

    /** Constructor for QgsFeature
     * @param id feature id
     */
    QgsFeature( QgsFeatureId id = QgsFeatureId() );

    /** Constructor for QgsFeature
     * @param fields feature's fields
     * @param id feature id
     */
    QgsFeature( const QgsFields &fields, QgsFeatureId id = QgsFeatureId() );

    /** Copy constructor
     */
    QgsFeature( const QgsFeature &rhs );

    /** Assignment operator
     */
    QgsFeature &operator=( const QgsFeature &rhs );

    /**
     * Compares two features
     */
    bool operator==( const QgsFeature &other ) const;

    /**
     * Compares two features
     */
    bool operator!=( const QgsFeature &other ) const;

    virtual ~QgsFeature();

    /** Get the feature ID for this feature.
     * @returns feature ID
     * @see setId()
     */
    QgsFeatureId id() const;

    /** Sets the feature ID for this feature.
     * @param id feature id
     * @see id
     */
    void setId( QgsFeatureId id );

    /** Returns the feature's attributes.
     * @link attributes @endlink method.
     * @returns list of feature's attributes
     * @see setAttributes
     * @note added in QGIS 2.9
     * @note Alternatively in Python: iterate feature, eg. @code [attr for attr in feature] @endcode
     */
    QgsAttributes attributes() const;

    /** Sets the feature's attributes.
     * The feature will be valid after.
     * @param attrs attribute list
     * @see setAttribute
     * @see attributes
     */
    void setAttributes( const QgsAttributes &attrs );

    /** Set an attribute's value by field index.
     * The feature will be valid if it was successful.
     * @param field the index of the field to set
     * @param attr the value of the attribute
     * @return false, if the field index does not exist
     * @note For Python: raises a KeyError exception instead of returning false
     * @note Alternatively in Python: @code feature[field] = attr @endcode
     * @see setAttributes
     */
    bool setAttribute( int field, const QVariant &attr );

    /** Initialize this feature with the given number of fields. Discard any previously set attribute data.
     * @param fieldCount Number of fields to initialize
     */
    void initAttributes( int fieldCount );

    /** Deletes an attribute and its value.
     * @param field the index of the field
     * @see setAttribute
     * @note For Python: raises a KeyError exception if the field is not found
     * @note Alternatively in Python: @code del feature[field] @endcode
     */
    void deleteAttribute( int field );

    /** Returns the validity of this feature. This is normally set by
     * the provider to indicate some problem that makes the feature
     * invalid or to indicate a null feature.
     * @see setValid
     */
    bool isValid() const;

    /** Sets the validity of the feature.
     * @param validity set to true if feature is valid
     * @see isValid
     */
    void setValid( bool validity );

    /** Returns true if the feature has an associated geometry.
     * @see geometry()
     * @note added in QGIS 3.0.
     */
    bool hasGeometry() const;

    /** Returns the geometry associated with this feature. If the feature has no geometry,
     * an empty QgsGeometry object will be returned.
     * @see hasGeometry()
     * @see setGeometry()
     */
    QgsGeometry geometry() const;

    /** Set the feature's geometry. The feature will be valid after.
     * @param geometry new feature geometry
     * @see geometry()
     * @see clearGeometry()
     */
    void setGeometry( const QgsGeometry &geometry );

    /** Removes any geometry associated with the feature.
     * @see setGeometry()
     * @see hasGeometry()
     * @note added in QGIS 3.0
     */
    void clearGeometry();

    /** Assign a field map with the feature to allow attribute access by attribute name.
     *  @param fields The attribute fields which this feature holds
     *  @param initAttributes If true, attributes are initialized. Clears any data previously assigned.
     *                        C++: Defaults to false
     *                        Python: Defaults to true
     * @note added in QGIS 2.9
     * @see fields
     */
    void setFields( const QgsFields &fields, bool initAttributes = false );

    /** Returns the field map associated with the feature.
     * @see setFields
     */
    QgsFields fields() const;

    /** Insert a value into attribute. Returns false if attribute name could not be converted to index.
     *  Field map must be associated using @link setFields @endlink before this method can be used.
     *  The feature will be valid if it was successful
     *  @param name The name of the field to set
     *  @param value The value to set
     *  @return false if attribute name could not be converted to index (C++ only)
     *  @note For Python: raises a KeyError exception instead of returning false
     *  @note Alternatively in Python: @code feature[name] = attr @endcode
     *  @see setFields
     */
    bool setAttribute( const QString &name, const QVariant &value );

    /** Removes an attribute value by field name. Field map must be associated using @link setFields @endlink
     *  before this method can be used.
     *  @param name The name of the field to delete
     *  @return false if attribute name could not be converted to index (C++ only)
     *  @note For Python: raises a KeyError exception instead of returning false
     *  @note Alternatively in Python: @code del feature[name] @endcode
     *  @see setFields
     */
    bool deleteAttribute( const QString &name );

    /** Lookup attribute value from attribute name. Field map must be associated using @link setFields @endlink
     *  before this method can be used.
     *  @param name The name of the attribute to get
     *  @return The value of the attribute (C++: Invalid variant if no such name exists )
     *  @note For Python: raises a KeyError exception if the field is not found
     *  @note Alternatively in Python: @code feature[name] @endcode
     *  @see setFields
     */
    QVariant attribute( const QString &name ) const;

    /** Lookup attribute value from its index. Field map must be associated using @link setFields @endlink
     *  before this method can be used.
     *  @param fieldIdx The index of the attribute to get
     *  @return The value of the attribute (C++: Invalid variant if no such index exists )
     *  @note For Python: raises a KeyError exception if the field is not found
     *  @note Alternatively in Python: @code feature[fieldIdx] @endcode
     *  @see setFields
     */
    QVariant attribute( int fieldIdx ) const;

    /** Utility method to get attribute index from name. Field map must be associated using @link setFields @endlink
     *  before this method can be used.
     *  @param fieldName name of field to get attribute index of
     *  @returns -1 if field does not exist or field map is not associated.
     *  @see setFields
     */
    int fieldNameIndex( const QString &fieldName ) const;

    //! Allows direct construction of QVariants from features.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

  private:

    QExplicitlySharedDataPointer<QgsFeaturePrivate> d;

}; // class QgsFeature

//! Writes the feature to stream out. QGIS version compatibility is not guaranteed.
CORE_EXPORT QDataStream &operator<<( QDataStream &out, const QgsFeature &feature );
//! Reads a feature from stream in into feature. QGIS version compatibility is not guaranteed.
CORE_EXPORT QDataStream &operator>>( QDataStream &in, QgsFeature &feature );

// key = feature id, value = changed attributes
typedef QMap<QgsFeatureId, QgsAttributeMap> QgsChangedAttributesMap;

// key = feature id, value = changed geometry
typedef QMap<QgsFeatureId, QgsGeometry> QgsGeometryMap;

typedef QSet<QgsFeatureId> QgsFeatureIds;

// key = field index, value = field name
typedef QMap<int, QString> QgsFieldNameMap;

typedef QList<QgsFeature> QgsFeatureList;

uint qHash( const QgsFeature &key, uint seed = 0 );

Q_DECLARE_METATYPE( QgsFeature )
Q_DECLARE_METATYPE( QgsFeatureList )

#endif

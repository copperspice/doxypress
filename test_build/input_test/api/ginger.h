/*************************************************************************
*
* Copyright (C) 2012-2018 Barbara Geller & Ansel Sermersheim
* All rights reserved.
*
* GNU Free Documentation License
*
*************************************************************************/

#ifndef GINGER_H
#define GINGER_H

#include <QList>
#include <QString>

class Ginger : public CSObject
{
   CS_OBJECT(Ginger)

   CS_ENUM(Spices)

   // title is type QString
   CS_PROPERTY_READ(title,        getTitle)
   CS_PROPERTY_WRITE(title,       setTitle)
   CS_PROPERTY_RESET(title,       resetTitle)
   CS_PROPERTY_NOTIFY(title,      titleChanged)
   CS_PROPERTY_REVISION(title,    31415)
   CS_PROPERTY_DESIGNABLE(title,  true)
   CS_PROPERTY_SCRIPTABLE(title,  isScriptTitle()   )
   CS_PROPERTY_STORED(title,      m_title.isEmpty() )
   CS_PROPERTY_USER(title,        10 > 100)
   CS_PROPERTY_CONSTANT(title)
   CS_PROPERTY_FINAL(title)

   // favorite is type enum Spices
   CS_PROPERTY_READ(favorite,   getFavorite)
   CS_PROPERTY_WRITE(favorite,  setFavorite)
   CS_PROPERTY_NOTIFY(favorite, ready)

   // testing Qt
   Q_OBJECT
   Q_ENUM(Spices)
   Q_PROPERTY(QString title_Qt  READ getTitle  WRITE setTitle)

   public:
      Ginger();
      ~Ginger();

      enum Spices { mint, basil, Salt, Pepper =100, cloves };
      enum Brands { Chinese, Jamaican, Thai, Yellow };

      // registration is only reguired if the enum is used in a proptery or a signal
      CS_REGISTER_ENUM( enum Shapes { square, triangle, circle, trapezoid, diamond=0x10 }; )

      // testing Qt
      enum Spice_Qt { Allspice, Mace };

      using AmountType = double;

      void setAmount(double amt);
      double getAmount();

      bool isGrated();
      bool isChopped();

      QList<int> takesRValue(QString && dataA);

      inline bool isPowder() {
         return false;
      }

      static bool isSpice();

      // (1) methods for title property
      QString getTitle() const {
         return m_title;
      }

      void setTitle(QString data) {
         m_title = data;
         emit titleChanged(m_title);
      }

      void resetTitle();

      CS_SIGNAL_1(Public, void titleChanged(QString title))
      CS_SIGNAL_2(titleChanged, title)

      // (2) methods for favorite property
      Spices getFavorite() const {
         return m_favorite;
      }

      void setFavorite(Spices data) {
         m_favorite = data;
      }

      CS_SLOT_1(Public, void newTitle(QString))
      CS_SLOT_2(newTitle)

      CS_SIGNAL_1(Public, void ready())
      CS_SIGNAL_2(ready)

   protected:
      QString someProtected();

      CS_SIGNAL_1(Protected, void test_B(int var_s[] ) )
      CS_SIGNAL_2(test_B, var_s)

      CS_SLOT_1(Protected, void someProtectedSlot() )
      CS_SLOT_2(someProtectedSlot)

      CS_SLOT_1(Protected, void test_D(int data[][42], void (*ptr)(int,QString *)))
      CS_SLOT_OVERLOAD(test_D,(int [][42], void (*)(int,QString *)))

   private:
      QString somePrivate();
      double m_amount;

      QString m_title;        // property
      Spices m_favorite;      // property

      bool isScriptTitle() const;

      CS_SLOT_1(Private, void somePrivateSlot(int temp) )
      CS_SLOT_2(somePrivateSlot)

      // more testing
      CS_SIGNAL_1(Private, void test_A( Ginger **var_r ) )
      CS_SIGNAL_2(test_A, var_r)

      CS_SLOT_1(Private, void test_C(bool b){setTitle("new title");})
      CS_SLOT_2(test_C)
};

Q_DECLARE_METATYPE(Ginger::Spices)
Q_DECLARE_TYPEINFO(QString, Q_MOVABLE_TYPE)
Q_DECLARE_SHARED(QString)
Q_DECLARE_OPERATORS_FOR_FLAGS(QString::SectionFlags)

class Lemongrass : public CSObject
{
   public:
      int howMuch;
      QString otherNames;

      /** This is the addHoliday. A bit more information about the method.  */
      void addHoliday(int dayNum, QString monthName, int year);

      /// This is the monthName method.
      void monthName(int value);

      int addData(QString textA, int valueB) {
         // do something
         return 42;
      };
}

#endif

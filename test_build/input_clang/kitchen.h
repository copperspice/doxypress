/*************************************************************************
*
* Copyright (C) 2012-2016 Barbara Geller & Ansel Sermersheim
* All rights reserved.
*
*
* GNU Free Documentation License
* This file may be used under the terms of the GNU Free Documentation
* License version 1.3 as published by the Free Software Foundation
* and appearing in the file included in the packaging of this file.
*
*
*************************************************************************/

/* the following two lines are includes */
#include <string>
#include <vector>

#include <QObject>
#include <QString>

/// document def for max
#define MAX 100

/// document a preprocessor directive, docs are missing
#ifdef __linux
   #define BUILD_LINUX
#endif

/// global vector to track number age limit of each event
std::vector<int> globalVector;

/// forward declaration documentation
class gameSports;

bool isTrailOpen(std::string name) {
   // comment in the body of a function
   bool retval = true;

   if (name == "Bear Mountain") {
      retval = false;
   }

   return retval;
}


/// templated function, method waterSport must be defined for each T
template <typename T>
bool isWaterSport()
{
   return T::waterSport();
}

class gameBase
{
   public:
      virtual void gameRequirements();
};

/// This is the detailed documentation for class gameSports.
class gameSports : public gameBase
{
   public:
      gameSports() = delete;
      explicit gameSports(int whichGame);
      gameSports(gameSports &&) = default;        ///< comment for the defaulted constructor
      ~gameSports();

      /// documentation for a nested class
      class gameSolo {
         public:
            std::string name;

         // this struct is a nested inside a class then inside another class
         struct gameSoloNested {
            bool isValidGame;
         };

      };

      /** (C-1)  comment before method, two stars */
      void gameRequirements() override;

      friend int kayakCapacity(int len, int width, const std::string &model);
      friend class Ginger;
      friend struct xGameSetup;

      float bikeSpeed(long bVar) &&;              /**< (C-2)  comment with two stars  */

      volatile int boatType(std::string &&var);   /*!< (C-3)  comment with one star and one exclamation  */

      const std::string horseBreed(int);          ///< (C-4)  comment with three slashes and a less than

      void fencing(int (*funcVar)(bool)) const;   //!< (C-5)  comment two slashes and one exclamation

      /// (C-6)  comment with three slashes, before the method
      void volleyball(float var1[5], const double var2 = 10.9, const int var3 = 15 > sizeof(int) ? 3 + 5 : false);

      void swim(int (gameSports::*dataVar)() const);

      virtual void playChess() final;
      virtual void playCheckers() = 0;

      inline void addEvent(int value) noexcept {
         numEvents += value;
      }

      /// This is the brief comment for method firstTime
      /** This is the detailed comment for method firstTime. More docs for the detailed. */

      static int firstTime();

      /// This is a member of the current class. It also has a detailed description.
      bool isSportOutside;

      enum class Locations { Denver, Aspen, Boulder };

      enum eventType { Summer,         ///< comment for the summer
                       Winter          ///< comment for the winter
      };

      /// documentation for templated method
      template<class T, class U = std::string, typename ...Vs>
      std::string eventSeating(T data, int var, U string)  noexcept(std::is_nothrow_move_constructible<T>::value);

      typedef int myInt;;        ///< typedef sample documentation (expression has two semicolons)
      using myBool = bool;       ///< using sample

      //  (C-7) comment with two slashes, should be ignored

      /// (C-8) comment with three slashes, should be ignored as it does not apply to anything

   protected:
      float poolLength();
      float poolDepth();

      double poolHeated(std::string id) volatile;

      mutable int maxPlayers;

   private:
      auto drugTesting(std::vector<int>) -> bool;

      static bool playersGender;
      int numEvents;
};


/// documentation for a templated class
template<class T>
class eventScoring
{
   public:
      bool isTimedEvent;
      bool isScoredEvent;
};


/// documentation for a templated specialization class
template<class T>
class eventScoring<T *>
{
   public:
      /// documentation for templated method in eVent Scoring
      template<typename ...Us>
      std::string eventPlannig(T data, Us... Vs);
};


/// documentation for a class which inherits from a templated class
class eventTimed : public eventScoring<int>
{
   public:
      constexpr int max() const volatile
         { return MAX; }                 ///< comment for constexpr

      double maxTime;
};


struct xGameSetup
{
   bool isBallRequired;
   int numberOfPlayers;
};

/// docs for struct using final
struct xGameFields final : xGameSetup
{
   bool isGrassy;

   /// nested structure comment
   struct gameFieldSize{
      int length;
      int width;
   };
};

/// here is a simple union
union Teamsters
{
   float timeMark;
   std::string raceName;
};

/// comment for a namespace
namespace CopperSpice{

/// Ginger is a class to test documenting CopperSpice specifics macros.
class Ginger : public QObject
{
   ///  macro expansion documentation
   CS_OBJECT(Ginger)

/*
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

   CS_ENUM(Spices)

   // registration is only required if the enum is used in a property or a signal
   CS_REGISTER_ENUM( enum Shapes { square, triangle, circle, trapezoid, diamond=0x10 }; )

   CS_SIGNAL_1(Public, void titleChanged(QString title))
   CS_SIGNAL_2(titleChanged, title)
*/

   public:
      enum Spices { mint, basil, Salt, Pepper =100, cloves };

      QString getTitle() const {
         return m_title;
      }

      void setTitle(QString data) {
         m_title = data;
      }

   private:
      QString m_title;

};

}


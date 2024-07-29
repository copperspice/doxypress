/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
*
* DoxyPress is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
*************************************************************************/

#ifndef KITCHEN_H
#define KITCHEN_H

/* Following lines are includes */
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <QObject>
#include <QString>

/// This is brief documentation for max variable which is a define
#define MAX 100

/*! Here we have detailed comment, macro will be expanded to return the smaller value which is either A or B */
#define MIN(a, b)    ((a < b ) ? (a) : (b))

/// This is brief for a multi line macro
#define MultiLines \
   do { \
   } while (false)

/// document a preprocessor directive, docs missing as they should be
#ifdef __linux
   #define BUILD_LINUX
#endif

/// global vector to track number age limit of each event
std::vector<int> globalVector;

/// Brief documentation, comment is located next to the forward declaration
class Clang_gameSports;

bool isTrailOpen(std::string name) {
   // comment in the body of a function
   bool retval = true;

   if (name == "Bear Mountain") {
      retval = false;
   }

   return retval;
}


/// Brief docs for isWaterSport method. This will start the detailed documentation for the templated
/// function. The method waterSport must be defined for each T.
template <typename T>
bool isWaterSport()
{
   return T::waterSport();
}

class Clang_gameBase
{
   public:
      virtual void gameRequirements();
};


//! Documentation for a concept (brief)

//! Documentation for a concept (full description). Refer to CS_Moveable for a longer description.
template <typename T1, class T2>
concept CS_IsClass = std::is_class_v<T1>;

/// Brief documentation for a moveable concept.
template <typename U>
concept CS_Moveable = std::is_move_constructible_v<U>;


/// This is a class to test functionality for C++20
template <typename X>
requires requires (X tmp) { tmp + tmp; }
class Concept_Test
{
   public:
      /// Template using a hand written concept
      template <CS_Moveable T>
      T moveMe_1(T varA);

      /// Template method with a requires clause
      template <typename T, typename U>
      requires CS_Moveable<T> && CS_Moveable<U>
      T moveMe_2(U varA);

      /// Requires clause is located at the end of the declaration
      template <typename T, typename U>
      T moveMe_3(U varA) requires CS_Moveable<T> && CS_Moveable<U>;

      /// Template using a hand written concept
      template <typename T>
      void makeFood(T varB);

      /// form (a) one for nodiscard
      [[nodiscard]] int thing_a1(bool varC);

      /// form (a) two for noreturn
      double thing_a2 [[noreturn]] (bool varD);

      /// form (a) three attribute deprecated
      [[deprecated]] void thing_a3 (bool varE);



      /** form (b) one for nodiscard */
      [[nodiscard]] int thing_b1(bool varC);

      /** form (b) two for noreturn  */
      double thing_b2 [[noreturn]] (bool varD);

      /** form (b) three attribute deprecated */
      [[deprecated]] void thing_b3 (bool varE);



      /*! form (c) one for nodiscard */
      [[nodiscard]] int thing_c1(bool varC);

      /*! form (c) two for noreturn  */
      double thing_c2 [[noreturn]] (bool varD);

      /*! (form (c) three  attribute deprecated */
      [[deprecated]] void thing_c4 (bool varE);



      //! form (d) one for nodiscard
      [[nodiscard]] int thing_d1(bool varC);

      //! form (d) two for noreturn
      double thing_d2 [[noreturn]] (bool varD);

      //! corm (c) three attribute deprecated
      [[deprecated]] void thing_d4 (bool varE);
};


/// This is the detailed documentation for class Clang_gameSports, located above the class declaration.
class Clang_gameSports : public Clang_gameBase
{
   public:
      Clang_gameSports() = delete;
      explicit Clang_gameSports(int whichGame);

      Clang_gameSports(const Clang_gameSports &) = default;     ///< Comment for a defaulted copy constructor
      Clang_gameSports(Clang_gameSports &&) = default;          ///< Comment for a defaulted move constructor

      /// Comment about destructor
      ~Clang_gameSports();

      Clang_gameSports &operator=(const Clang_gameSports &);      ///< Comment for a copy assignment
      Clang_gameSports &operator=(Clang_gameSports &&);           ///< Comment for a move assignment

      /// Documentation for a nested class
      class gameSolo {
         public:
            std::string name;

         // This struct is a nested inside a class then inside another class
         struct gameSoloNested {
            bool isValidGame;
         };

      };

      /** (C-1)  comment before method, two stars */
      void gameRequirements() override;

      friend int kayakCapacity(int len, int width, const std::string &model);
      friend class AfterParty;
      friend struct Clang_struct_abstract;

      float bikeSpeed(long bVar) &&;                  /**< (C-2)  comment with two stars  */

      volatile int boatType(std::string &&var);       /*!< (C-3)  comment with one star and one exclamation  */

      const std::string horseBreed(int);              ///< (C-4)  comment with three slashes and a less than

      void fencing(int (*funcVar)(bool)) const;       //!< (C-5)  comment two slashes and one exclamation

      /// (C-6)  comment with three slashes, before the method
      void volleyball(const double var1 = 10.9, const int var2 = 15 > sizeof(int) ? 3 + 5 : false);

      void volleyball(float var1[5][7], float var2[9]);

      void swim(int (Clang_gameSports::*dataVar)() const);

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


      /// With extract all turned on the enums will show in a table with no documentation
      enum class Locations { Denver, Aspen, Boulder };

      /// Document the eventType enum (brief).  This should be the main docs for eventType.
      enum eventType { Summer,                        ///< comment for the summer
                       Winter                         ///< comment for the winter
      };

      /// documentation for templated method
      template<class T, class U = std::string, typename ...Vs>
      std::string eventSeating(T data, int var, U string)  noexcept(std::is_nothrow_move_constructible<T>::value);

      typedef int myInt;;                            ///< Documentation for a typedef (myInt expression has two semicolons)

      // Docs here are just in case the two semicolons mess up parsing
      void dummy();

      using myBool = bool;                           ///< These are sample docs for a using.

      // C++14 syntax
      auto hiking() {                                ///< Return type of this method is auto which should be deduced as an int.
         return 42;
      }

      // C++17 syntax
      char32_t myLetter();                           ///< New syntaxa as of C++17

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
      int villageStores[42][17][8];

      // C++17 syntax
      static inline int trail_Length = 27;            ///< Can we have an inline variable?

      std::variant<int, bool, float> someSize;        ///< Declaration for a variant. This data type is part of C++17

      static const constexpr auto somePair = std::pair(8, 17);  ///< Declare a pair without having to specify the data types
};


/// Documentation for a normal templated class
template<class T>
class Clang_eventScoring
{
   public:
      bool isTimedEvent;
      bool isScoredEvent;
};


/// Documentation for a partially specialized templated class
template<class T>
class Clang_eventScoring<T *>
{
   public:
      /// Documentation for a templated method of a partially specialized class
      template<typename ...Us>
      std::string eventPlanning(T data, Us... Vs);
};


/// Documentation for a fully specialized templated class
template< >
class Clang_eventScoring<bool>
{
   public:
      /// documentation for the bool version, which is a fully specialized templated class
      int maxTwoPlayers();
};


/// Documentation for a class which inherits from a templated class
class Clang_eventTimed : public Clang_eventScoring<int>
{
   public:
      constexpr int max() const volatile
         { return MAX; }                 ///< comment for constexpr

      double maxTime;
};


struct Clang_struct_abstract
{
   bool isBallRequired;
   int numberOfPlayers;
};

/// Documentation for a struct using final
struct Clang_structFields final : Clang_struct_abstract
{
   bool isGrassy;

   /// nested structure comment
   struct structFieldSize {
      int length;
      int width;
   };
};

/// Simple union
union Clang_team_union
{
   float timeMark;
   std::string raceName;
};

/// Comment for a namespace
namespace ns_clangTest {

/// %Afterparty is a class to test documenting macros.
class AfterParty : public QObject
{
   ///  macro expansion documentation
   CS_OBJECT(AfterParty)

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
      AfterParty() = default;                       ///< Defaulted constructor documentation.

      /// Information about the spices enum.
      enum Spices { mint, basil, Salt, Pepper =100, cloves };

      /// Used to get the current title.
      QString getTitle() const {
         return m_title;
      }

      /// Used to set the title
      void setTitle(QString data) {
         m_title = data;
      }

   private:
      QString m_title;
};

}


/// Brief for the free function (one).

/// Documentation for a free function ONE
void some1_FreeFunction();


/// Brief for the free function (two).

/// Documentation for a free function  TWO
void some2_FreeFunction();



/*!
\concept CS_Moveable
\brief This is second (brief) comment for the CS_Moveable concept.

The full documentation for the CS_Moveable concept is located in the external docs. Continued information to
show more details about this concept.


\sa Clang_eventTimed

*/

#endif
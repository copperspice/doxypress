/*************************************************************************
*
* Copyright (C) 2012-2018 Barbara Geller & Ansel Sermersheim
* All rights reserved.
*
* GNU Free Documentation License
*
*************************************************************************/

/* Following two lines are includes */
#include <string>
#include <utility>
#include <variant>
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

/// Brief documentation, comment is located next to the forward declaration
class cl_gameSports;

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

class cl_gameBase
{
   public:
      virtual void gameRequirements();
};

/// This is the detailed documentation for class cl_gameSports, located above the class declaration.
class cl_gameSports : public cl_gameBase
{
   public:
      cl_gameSports() = delete;
      explicit cl_gameSports(int whichGame);

      cl_gameSports(const cl_gameSports &) = default;       ///< Comment for a defaulted copy constructor
      cl_gameSports(cl_gameSports &&) = default;            ///< Comment for a defaulted move constructor

      /// Comment about destructor
      ~cl_gameSports();

      cl_gameSports &operator=(const cl_gameSports &);      ///< Comment for a copy assignment
      cl_gameSports &operator=(cl_gameSports &&);           ///< Comment for a move assignment

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
      friend struct cl_struct_abstract;

      float bikeSpeed(long bVar) &&;                  /**< (C-2)  comment with two stars  */

      volatile int boatType(std::string &&var);       /*!< (C-3)  comment with one star and one exclamation  */

      const std::string horseBreed(int);              ///< (C-4)  comment with three slashes and a less than

      void fencing(int (*funcVar)(bool)) const;       //!< (C-5)  comment two slashes and one exclamation

      /// (C-6)  comment with three slashes, before the method
      void volleyball(const double var1 = 10.9, const int var2 = 15 > sizeof(int) ? 3 + 5 : false);

      void volleyball(float var1[5][7], float var2[9]);

      void swim(int (cl_gameSports::*dataVar)() const);

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

      /// Document the eventType enum
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
      char32_t myLetter();                           ///< Getting ready for C++17

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
      static inline int trailLength = 27;             ///< Can we have an inline variable?

      std::variant<int, bool, float> someSize;        ///< Declaration for a variant. This data type is part of C++17

      static const constexpr auto somePair = std::pair(8, 17);  ///< Declare a pair without having to specify the data types
};


/// documentation for a normal templated class
template<class T>
class cl_eventScoring
{
   public:
      bool isTimedEvent;
      bool isScoredEvent;
};


/// documentation for a partially specialized templated class
template<class T>
class cl_eventScoring<T *>
{
   public:
      /// Documentation for a templated method of a partially specialized class
      template<typename ...Us>
      std::string eventPlanning(T data, Us... Vs);
};


/// documentation for a fully specialized templated class
template< >
class cl_eventScoring<bool>
{
   public:
      /// documentation for the bool version, which is a fully specialized templated class
      int maxTwoPlayers();
};


/// documentation for a class which inherits from a templated class
class cl_eventTimed : public cl_eventScoring<int>
{
   public:
      constexpr int max() const volatile
         { return MAX; }                 ///< comment for constexpr

      double maxTime;
};


struct cl_struct_abstract
{
   bool isBallRequired;
   int numberOfPlayers;
};

/// docs for struct using final
struct cl_structFields final : cl_struct_abstract
{
   bool isGrassy;

   /// nested structure comment
   struct structFieldSize {
      int length;
      int width;
   };
};

/// here is a simple union
union cl_team_union
{
   float timeMark;
   std::string raceName;
};

/// comment for a namespace
namespace CL {

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

      /// Used to set the title.
      void setTitle(QString data) {
         m_title = data;
      }

   private:
      QString m_title;
};

}

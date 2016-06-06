#ifndef INCLUDED_MYCLASS
#define INCLUDED_MYCLASS

class test_GroupMethod
{
   public:
       /** comment A */
       std::string getSomething_A(void) const;
   
       /** comment C */
       bool isEnglish_C(void) const;
      
       /**
       * This is the comment for setX_NoGrouping.
       * @param bmyflag    This method should not be part of group two
       */
       void setX_NoGrouping(const bool bmyflag);
   
       void setSomethingElse_D(SomethingElse *somethingElse) { }
       SomethingElse * getSomethingElse_B(void) const { return 0; }   

   
       /** @name Group 1
       * The following one method is in group one
       */
       ///@{
       virtual void setStatusFlag (const std::string &f);
       ///@}
   
       /** @name Group 2
       *  The following four methods are in group two
       */
       ///@{
       ClassOne& toString(ClassOne &c) const;
       const ClassTwo& getClassTwo(void) const;
       void updateClassThree(const ClassThree *object) {}
       void getStuff(ClassFour &four, ClassFive *five) const;
       ///@}
       
};

#endif

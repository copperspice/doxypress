/*************************************************************************
*
* Copyright (C) 2012-2020 Barbara Geller & Ansel Sermersheim
*
* GNU Free Documentation License
*
*************************************************************************/

/** @defgroup group_a Group A
 *  Entries in group A
 *  @{
 */

/** @brief  Brief description of a function in group A */
void func_a1() {}

/** Another function in group A */
void func_a2() {}

/** @} */ // end of group A



/**
 *  @defgroup group_b Group B
 *  Entries in group B
 */



/** @defgroup group_c Group C
 *  Entries in group C
 */



/** @defgroup group_d Group D
 *  @ingroup group_c
 *  Group D is a child of group C
 */



/**
 *  @ingroup group_b
 *  @brief Function in group B
 */
void func_b1() {};

/** @ingroup group_b
 *  @brief Function in group B
 */
void func_b2() {};



/** @ingroup group_c
 *  @brief Function in @link group_c Group C@endlink.
 */
void func_c1() {};



/** @ingroup group_a group_b group_c group_d
 *  Namespace groupNS is in groups A, B, C, D
 *
 */

namespace groupNS{
   void func_all() {};
}




/** @file
 *  @ingroup group_c
 *  @brief this file in group C
 */



/** @defgroup group_e Group E
 *  Entries in group E
 *  @{
 */


/** @page mypage1 Section Group E Heading
 *  Additional documentation which will appear in the detailed description.
 */

/** @brief Function in group E
 */
void func_e() {};

/** @} */ // end of group_e



/** @addtogroup group_a
 *
 *  More documentation for group A.
 *  @{
 */

/** another function in group A */
void func_a3() {}

/** yet another function in group A */
void func_a4() {}

/** @} */ // end of group_a


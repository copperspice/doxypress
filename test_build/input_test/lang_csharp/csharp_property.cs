/************************************************************************
*
* Copyright (c) 2014-2025 Barbara Geller & Ansel Sermersheim
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

/// <summary>
/// Test properties in C#
/// </summary>
public class csharp_property
{
   /// <summary>
   /// First test: static property, default 55
   /// </summary>
   public int a { get; set; } = 55;

   /// <summary>
   /// Second test: non-static property, default 5.0
   /// </summary>
   public double b { get; set; } = 5.0;

   /// <summary>
   /// Third test: char, default 'c'
   /// </summary>
   public char c {get; set;} = 'c';

   /// <summary>
   /// Fourth test: byte, no default
   /// <summary>
   public byte d {get;set;}

   /// <summary>
   /// Main function
   /// </summary>
   /// <param name="args">Command-line arguments</param>
   public void Main(string[] args)
   {

   }
}
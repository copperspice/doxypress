/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
 *
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef DOT_H
#define DOT_H

#include <QByteArray>
#include <QList>
#include <QHash>
#include <QMutex>
#include <QQueue>
#include <QTextStream>
#include <QThread>
#include <QWaitCondition>

#include <stringmap.h>
#include <sortedlist.h>

class ClassDef;
class FileDef;
class ClassSDict;
class MemberDef;
class MemberList;
class Definition;
class DirDef;
class GroupDef;
class DotGroupCollaboration;
class DotRunnerQueue;

enum GraphOutputFormat    { GOF_BITMAP, GOF_EPS };
enum EmbeddedOutputFormat { EOF_Html, EOF_LaTeX, EOF_Rtf, EOF_DocBook };

/** Attributes of an edge of a dot graph */
struct EdgeInfo {
   enum Colors { Blue = 0, Green = 1, Red = 2, Purple = 3, Grey = 4, Orange = 5, Orange2 = 6 };
   enum Styles { Solid = 0, Dashed = 1 };

   EdgeInfo() : m_color(0), m_style(0), m_labColor(0) {}
   ~EdgeInfo() {}

   int m_color;
   int m_style;
   QString m_label;
   QString m_url;
   int m_labColor;
};

/** A node in a dot graph */
class DotNode
{
 public:
   enum GraphType { Dependency, Inheritance, Collaboration, Hierarchy, CallGraph };
   enum TruncState { Unknown, Truncated, Untruncated };

   DotNode(int n, const QString &label, const QString &tip, const QString &url,
           bool rootNode = false, QSharedPointer<ClassDef> cd = QSharedPointer<ClassDef>());

   ~DotNode();

   void addChild(DotNode *n, int edgeColor = EdgeInfo::Purple, int edgeStyle = EdgeInfo::Solid,
                 const QString &edgeLab = QString(), const QString &edgeURL = QString(), int edgeLabCol = -1);

   void addParent(DotNode *n);
   void deleteNode(SortedList<DotNode *> &deletedList, StringMap<QSharedPointer<DotNode>> *skipNodes = 0);
   void removeChild(DotNode *n);
   void removeParent(DotNode *n);
   int findParent( DotNode *n );

   void write(QTextStream &t, GraphType gt, GraphOutputFormat f,
              bool topDown, bool toChildren, bool backArrows, bool reNumber);

   int  m_subgraphId;
   void clearWriteFlag();
   void writeXML(QTextStream &t, bool isClassGraph);
   void writeDocbook(QTextStream &t, bool isClassGraph);
   void writeDEF(QTextStream &t);

   QString label() const {
      return m_label;
   }

   int  number() const {
      return m_number;
   }

   bool isVisible() const {
      return m_visible;
   }

   TruncState isTruncated() const {
      return m_truncated;
   }

   int distance() const {
      return m_distance;
   }

 private:
   void colorConnectedNodes(int curColor);
   void writeBox(QTextStream &t, GraphType gt, GraphOutputFormat f, bool hasNonReachableChildren, bool reNumber = false);

   void writeArrow(QTextStream &t, GraphType gt, GraphOutputFormat f, DotNode *cn,
                   EdgeInfo *ei, bool topDown, bool pointBack = true, bool reNumber = false);

   void setDistance(int distance);
   const DotNode  *findDocNode() const; // only works for acyclic graphs!

   void markAsVisible(bool b = true) {
      m_visible = b;
   }

   void markAsTruncated(bool b = true) {
      m_truncated = b ? Truncated : Untruncated;
   }

   int           m_number;
   QString       m_label;     //!< label text
   QString       m_tooltip;   //!< node's tooltip
   QString       m_url;       //!< url of the node (format: remote$local)

   QList<DotNode *>  *m_parents;   //!< list of parent nodes (incoming arrows)
   QList<DotNode *>  *m_children;  //!< list of child nodes (outgoing arrows)

   QList<EdgeInfo *> *m_edgeInfo;  //!< edge info for each child

   bool             m_deleted;   //!< used to mark a node as deleted
   bool             m_written;   //!< used to mark a node as written
   bool             m_hasDoc;    //!< used to mark a node as documented
   bool             m_isRoot;    //!< indicates if this is a root node

   QSharedPointer<ClassDef> m_classDef;  //!< class representing this node (can be 0)

   bool             m_visible;   //!< is the node visible in the output
   TruncState       m_truncated; //!< does the node have non-visible children/parents
   int              m_distance;  //!< shortest path to the root node

   friend class DotGfxHierarchyTable;
   friend class DotClassGraph;
   friend class DotInclDepGraph;
   friend class DotCallGraph;
   friend class DotGroupCollaboration;

   friend QString computeMd5Signature(DotNode *root, GraphType gt, GraphOutputFormat f, bool lrRank, bool renderParents,
                        bool backArrows, const QString &title, QString &graphStr);
};

inline int DotNode::findParent( DotNode *n )
{
   if (! m_parents) {
      return -1;
   }

   return m_parents->indexOf(n);
}

/** Represents a graphical class hierarchy */
class DotGfxHierarchyTable
{
 public:
   DotGfxHierarchyTable();
   ~DotGfxHierarchyTable();
   void writeGraph(QTextStream &t, const QString &path, const QString &fileName) const;

 private:
   void addHierarchy(DotNode *n, QSharedPointer<ClassDef> cd, bool hide);
   void addClassList(ClassSDict *cl);

   QList<DotNode *>          *m_rootNodes;
   QHash<QString, DotNode *> *m_usedNodes;

   static int    m_curNodeNumber;
   SortedList<DotNode *>  *m_rootSubgraphs;
};

/** Representation of a class inheritance or dependency graph */
class DotClassGraph
{
 public:
   DotClassGraph(QSharedPointer<ClassDef> cd, DotNode::GraphType t);
   ~DotClassGraph();

   bool isTrivial() const;
   bool isTooBig() const;

   QString writeGraph(QTextStream &t, GraphOutputFormat gf, EmbeddedOutputFormat ef,
                         const QString &path, const QString &fileName, const QString &relPath,
                         bool TBRank = true, bool imageMap = true, int graphId = -1) const;

   void writeXML(QTextStream &t);
   void writeDocbook(QTextStream &t);
   void writeDEF(QTextStream &t);
   QString diskName() const;

 private:
   void buildGraph(QSharedPointer<ClassDef> cd, DotNode *n, bool base, int distance);
   bool determineVisibleNodes(DotNode *rootNode, int maxNodes, bool includeParents);
   void determineTruncatedNodes(QList<DotNode *> &queue, bool includeParents);

   void addClass(QSharedPointer<ClassDef> cd, DotNode *n, int prot, const QString &label,
                 const QString &usedName, const QString &templSpec, bool base, int distance);

   DotNode *m_startNode;
   QHash<QString, DotNode *> *m_usedNodes;

   static int m_curNodeNumber;
   DotNode::GraphType m_graphType;
   QString m_diskName;
   bool m_lrRank;
};

/** Representation of an include dependency graph */
class DotInclDepGraph
{
 public:
   DotInclDepGraph(QSharedPointer<FileDef> fd, bool inverse);
   ~DotInclDepGraph();
   QString writeGraph(QTextStream &t, GraphOutputFormat gf, EmbeddedOutputFormat ef,
                         const QString &path, const QString &fileName, const QString &relPath,
                         bool writeImageMap = true, int graphId = -1) const;

   bool isTrivial() const;
   bool isTooBig() const;
   QString diskName() const;
   void writeXML(QTextStream &t);
   void writeDocbook(QTextStream &t);


 private:
   void buildGraph(DotNode *n, QSharedPointer<FileDef> fd, int distance);
   void determineVisibleNodes(QList<DotNode *> &queue, int &maxNodes);
   void determineTruncatedNodes(QList<DotNode *> &queue);

   DotNode                   *m_startNode;
   QHash<QString, DotNode *> *m_usedNodes;

   static int m_curNodeNumber;
   QString m_diskName;
   bool    m_inverse;
};

/** Representation of an call graph */
class DotCallGraph
{
 public:
   DotCallGraph(QSharedPointer<MemberDef> md, bool inverse);
   ~DotCallGraph();

   QString writeGraph(QTextStream &t, GraphOutputFormat gf, EmbeddedOutputFormat ef,
                         const QString  &path, const QString &fileName, const QString  &relPath,
                         bool writeImageMap = true, int graphId = -1) const;

   void buildGraph(DotNode *n, QSharedPointer<MemberDef> md, int distance);
   bool isTrivial() const;
   bool isTooBig() const;
   void determineVisibleNodes(QList<DotNode *> &queue, int &maxNodes);
   void determineTruncatedNodes(QList<DotNode *> &queue);

 private:
   DotNode        *m_startNode;
   static int      m_curNodeNumber;

   QHash<QString, DotNode *> *m_usedNodes;

   bool    m_inverse;
   QString m_diskName;

   QSharedPointer<Definition> m_scope;
};

/** Representation of an directory dependency graph */
class DotDirDeps
{
 public:
   DotDirDeps(QSharedPointer<DirDef> dir);
   ~DotDirDeps();

   bool isTrivial() const;
   QString writeGraph(QTextStream &out, GraphOutputFormat gf, EmbeddedOutputFormat ef, const QString  &path,
                         const QString &fileName, const QString  &relPath, bool writeImageMap = true, int graphId = -1) const;
 private:
   QSharedPointer<DirDef> m_dir;
};

/** Representation of a group collaboration graph */
class DotGroupCollaboration
{
 public :
   enum EdgeType {
      tmember = 0,
      tclass,
      tnamespace,
      tfile,
      tpages,
      tdir,
      thierarchy
   };

   class Link
   {
    public:
      Link(const QString d1, const QString &d2) : label(d1), url(d2) {}

      QString label;
      QString url;
   };

   class Edge
   {
    public :
      Edge(DotNode *start, DotNode *end, EdgeType type)
         : pNStart(start), pNEnd(end), eType(type)
      {
      }

      DotNode *pNStart;
      DotNode *pNEnd;
      EdgeType eType;

      QList<Link *> m_links;
      void write( QTextStream &t ) const;
   };

   DotGroupCollaboration(QSharedPointer<GroupDef> gd);
   ~DotGroupCollaboration();
   QString writeGraph(QTextStream &t, GraphOutputFormat gf, EmbeddedOutputFormat ef,
                         const QString &path, const QString &fileName, const QString &relPath,
                         bool writeImageMap = true, int graphId = -1) const;

   void buildGraph(QSharedPointer<GroupDef> gd);
   bool isTrivial() const;

 private :
   void addCollaborationMember(QSharedPointer<Definition> def, const QString &url, EdgeType eType);

   void addMemberList(QSharedPointer<MemberList> ml);
   void writeGraphHeader(QTextStream &t, const QString &title) const;

   Edge *addEdge(DotNode *_pNStart, DotNode *_pNEnd, EdgeType _eType, const QString &_label, const QString &_url );

   DotNode        *m_rootNode;
   int             m_curNodeId;

   QHash<QString, DotNode *> *m_usedNodes;

   QString         m_diskName;
   QList<Edge *>   m_edges;
};

/** Helper class to run dot
 */
class DotRunner
{
 public:
   struct CleanupItem {
      QString path;
      QString file;
   };

   /** Creates a runner for a dot \a file. */
   DotRunner(const QString &file, const QString &fontPath, bool checkResult, const QString &imageName = QString());

   /** Adds an additional job to the run.
    *  Performing multiple jobs one file can be faster.
    */
   void addJob(const QString &format, const QString &output);
   void addPostProcessing(const QString &cmd, const QString &args);

   void preventCleanUp() {
      m_cleanUp = false;
   }

   /** Runs dot for all jobs added. */
   bool run();

   CleanupItem cleanup() const {
      return m_cleanupItem;
   }

 private:
   QList<QString> m_jobs;

   QString m_postArgs;
   QString m_postCmd;

   QString m_file;
   QString m_path;
   QString m_dotExe;

   QString m_imageName;
   QString m_imageFormat;

   bool m_checkResult;
   bool m_cleanUp;
   bool m_multiTargets;

   CleanupItem m_cleanupItem;

   static std::mutex m_output_mutex;
};

/** class to insert a set of map files into an output file */
class DotFilePatcher
{
 public:
   struct Map {
      QString    mapFile;
      QString    relPath;
      bool       urlOnly;
      QString    context;
      QString    label;
      bool       zoomable;
      int        graphId;
   };

   DotFilePatcher(const QString &patchFile);

   int addMap(const QString &mapFile, const QString &relPath, bool urlOnly, const QString &context, const QString &label);
   int addFigure(const QString &baseName, const QString &figureName, bool heightCheck);
   int addSVGConversion(const QString &relPath, bool urlOnly, const QString &context, bool zoomable, int graphId);
   int addSVGObject(const QString &baseName, const QString &absImgName, const QString &relPath);

   bool run();
   QString file() const;

 private:
   QList<Map> m_maps;
   QString m_patchFile;
};

/** Queue of dot jobs to run. */
class DotRunnerQueue
{
 public:
   void enqueue(DotRunner *runner);
   DotRunner *dequeue();
   uint count() const;

 private:
   QWaitCondition  m_bufferNotEmpty;
   QQueue<DotRunner *> m_queue;
   mutable QMutex  m_mutex;
};

/** Worker thread to execute a dot run */
class DotWorkerThread : public QThread
{
 public:
   DotWorkerThread(DotRunnerQueue *queue);
   void run();
   void cleanup();

 private:
   DotRunnerQueue *m_queue;
   QList<DotRunner::CleanupItem> m_cleanupItems;
};

/** Singleton that manages dot relation actions */
class DotManager
{
 public:
   static DotManager *instance();
   void addRun(DotRunner *run);

   int  addMap(const QString &file, const QString &mapFile, const QString &relPath, bool urlOnly,
                  const QString &context, const QString &label);

   int addFigure(const QString &file, const QString &baseName, const QString &figureName, bool heightCheck);

   int addSVGConversion(const QString &file, const QString &relPath,
                  bool urlOnly, const QString &context, bool zoomable, int graphId);

   int addSVGObject(const QString &file, const QString &baseName, const QString &figureName, const QString &relPath);

   bool run();

 private:
   DotManager();
   virtual ~DotManager();

   QList<DotRunner *> m_dotRuns;

   StringMap<QSharedPointer<DotFilePatcher>> m_dotMaps;

   static DotManager        *m_theInstance;
   DotRunnerQueue           *m_queue;
   QList<DotWorkerThread *>  m_workers;
};


/** Generated a graphs legend page */
void generateGraphLegend(const QString &path);

void writeDotGraphFromFile(const QString &inFile, const QString &outDir, const QString &outFile, GraphOutputFormat format);

void writeDotImageMapFromFile(QTextStream &t, const QString &inFile, const QString &outDir, const QString &relPath,
                  const QString &baseName, const QString &context, int graphId = -1);

void writeDotDirDepGraph(QTextStream &t, QSharedPointer<DirDef> dd);

#endif

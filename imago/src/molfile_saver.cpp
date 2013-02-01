/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
 * 
 * This file is part of Imago toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include <cstdio>
#include <ctime>
#include <map>

#include "boost/graph/iteration_macros.hpp"

#include "label_combiner.h"
#include "molecule.h"
#include "molfile_saver.h"
#include "output.h"
#include "skeleton.h"
#include "superatom.h"
#include "settings.h"
#include "log_ext.h"

using namespace imago;

MolfileSaver::MolfileSaver( Output &out ) : _mol(0), _out(out)
{
}

MolfileSaver::~MolfileSaver()
{
}

void MolfileSaver::saveMolecule(const Settings& vars, const Molecule &mol )
{
   _mol = &mol;

   _writeHeader();
   _writeCtab(vars);
   _out.writeStringCR("M  END");
}

void MolfileSaver::_writeHeader()
{
   time_t tm = time(NULL);
   const struct tm *lt = localtime(&tm);

   _out.writeCR();
   _out.printf("  -IMAGO- %02d%02d%02d%02d%02d2D\n", lt->tm_mon + 1, lt->tm_mday,
      lt->tm_year % 100, lt->tm_hour, lt->tm_min);
   _out.writeCR();
   _out.printf("%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d V3000\n", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}


void MolfileSaver::_writeCtab(const Settings& vars)
{
	logEnterFunction();

   const Skeleton::SkeletonGraph &graph = _mol->getSkeleton();
   const Molecule::ChemMapping &labels = _mol->getMappedLabels();
   std::map<Skeleton::Vertex, int> mapping;
   _out.writeStringCR("M  V30 BEGIN CTAB");
   _out.printf("M  V30 COUNTS %d %d 0 0 0\n", boost::num_vertices(graph), 
      boost::num_edges(graph));
   _out.writeStringCR("M  V30 BEGIN ATOM");

   int i = 1, j;
   char label[3] = {0, 0, 0};
   double bond_length;

   bond_length = vars.dynamic.AvgBondLength;

   BGL_FORALL_VERTICES(v, graph, Skeleton::SkeletonGraph)
   {
      _out.printf("M  V30 %d ", i);
      mapping[v] = i;
      Molecule::ChemMapping::const_iterator it = labels.find(v);

      const Superatom *satom;

      if (it == labels.end())
      {
         satom = 0;
         label[0] = 'C';
         label[1] = 0;
         _out.printf("%s", label);
      }
      else
      {
         satom = &(it->second->satom);

         if (satom->atoms.size() == 1)
         {
			 Atom atom = satom->atoms[0];
			 label[0] = atom.getLabelFirst();
			 label[1] = atom.getLabelSecond();
            _out.printf("%s", label);
			
			getLogExt().append("Label", label);
			
			// R-groups used different store notation
			if (atom.getLabelFirst() == 'R' && atom.getLabelSecond() == 0)
            {
				if (atom.charge > 0)
				{
					getLogExt().append("R-group index", atom.charge);
					_out.printf("%d", atom.charge);
				}
				else
				{
					getLogExt().appendText("R-group index=1");
					_out.printf("#");
				}
            }
         }
         else
         {
            for (j = 0; j != satom->atoms.size(); j++)
            {
               const Atom &atom = satom->atoms[j];

               if (atom.isotope > 0)
               {
                  _out.printf("\\S%d", atom.isotope);
               }
			   label[0] = atom.getLabelFirst();
			   label[1] = atom.getLabelSecond();
               _out.printf("%s", label);
               if (atom.count > 1)
               {
                  _out.printf("%d", atom.count);
               }
			   if (atom.getLabelFirst() == 'R' && atom.getLabelSecond() == 0 && atom.charge != 0)
               {
                  _out.printf("\\S%d", atom.charge);
               }
               else
               {
                  if (atom.charge > 0)
                  {
                     _out.printf("\\S%d+", atom.charge);
                  }
                  else if (atom.charge < 0)
                  {
                     _out.printf("\\S%d-", -atom.charge);
                  }
               }
            }
         }
      }

      Vec2d vert_pos = boost::get(boost::vertex_pos, graph, v);

      if (!satom)
         _out.printf(" %lf %lf 0 0", vert_pos.x / bond_length, -vert_pos.y / bond_length);
      else
      {
         const Label &l = *(labels.find(v))->second;
         double x = l.rect.x + l.rect.width / 2.0;
         double y = l.rect.y + l.rect.height / 2.0;
         if (l.multiline)
            _out.printf(" %lf %lf 0 0", vert_pos.x / bond_length, -vert_pos.y / bond_length);
         else
            _out.printf(" %lf %lf 0 0", x / bond_length, -y / bond_length);

         if (satom->atoms.size() == 1)
         {
			 if (satom->atoms[0].charge != 0 && satom->atoms[0].getLabelFirst() != 'R')
               _out.printf(" CHG=%d", satom->atoms[0].charge);
            if (satom->atoms[0].isotope > 0)
               _out.printf(" MASS=%d", satom->atoms[0].isotope);
         }
      }

      _out.writeCR();
      i++;
   }

   _out.writeStringCR("M  V30 END ATOM");
   _out.writeStringCR("M  V30 BEGIN BOND");

   j = 1;
   BGL_FORALL_EDGES(e, graph, Skeleton::SkeletonGraph)
   {
      int type;
      const Bond bond = boost::get(boost::edge_type, graph, e);

      type = bond.type;

      int begin = mapping.find(boost::source(e, graph))->second,
          end = mapping.find(boost::target(e, graph))->second;

      if (type == BT_SINGLE_DOWN || type == BT_SINGLE_UP)
         _out.printf("M  V30 %d %d %d %d", j++, BT_SINGLE, begin, end);
      else
         _out.printf("M  V30 %d %d %d %d", j++, type, begin, end);

	  // line endings "\n" here are platform-indepent and fixed.

      switch (type)
      {
      case BT_SINGLE_UP:
         _out.printf(" CFG=1\n");
         break;
      case BT_SINGLE_DOWN:
         _out.printf(" CFG=3\n");
         break;
      default:
         _out.printf("\n");
         break;
      }
   }
   _out.writeStringCR("M  V30 END BOND");
   _out.writeStringCR("M  V30 END CTAB");
}

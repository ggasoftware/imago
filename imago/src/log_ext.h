/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
 *
 * This file is part of Imago OCR project.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#pragma once
#ifndef _log_ext_h
#define _log_ext_h

#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include "stl_fwd.h"
#include "segment.h"
#include "skeleton.h"
#include "segments_graph.h"
#include "image_utils.h"
#include "image_draw_utils.h"
#include "virtual_fs.h"
#include "settings.h"

#define logEnterFunction imago::log_ext_service::LogEnterFunction _entry(__FUNCTION__, imago::getLogExt()); _entry._logEnterFunction


namespace imago
{
	class log_ext;

	log_ext& getLogExt();

	struct ProfilingInformation
	{
		unsigned int calls;
		unsigned int totalTime;
		int maxMemory;
		ProfilingInformation();
	};

	struct FunctionRecord
	{
		std::string name;
		std::string anchor;
		unsigned int memory;
		unsigned int time_start;
		unsigned int time_log_ms;
		FunctionRecord(const std::string& n);
		std::string getPlatformSpecificInfo();
		unsigned int getTotalTime();
		unsigned int getWorkTime();
		unsigned int getLogTime();
		int getMemDelta();
	};

	class log_ext
	{
	public:
		log_ext(const std::string folder);
		virtual ~log_ext();

		void SetVirtualFS(VirtualFS& vfs)
		{
			pVFS = &vfs;
			UseVirtualFS = true;
		}

      void SetNoVirtualFS()
      {
         pVFS = NULL;
         UseVirtualFS = false;
      }

		bool loggingEnabled() const;
		void setLoggingEnabled(bool value);

		template <class t> void append(const std::string& name, const t& value)
		{
			if(!loggingEnabled()) return;

			std::ostringstream out;
			out << value;

			dump(getStringPrefix() + "<i>" + filterHtml(name) + "</i>: <u>" + filterHtml(out.str()) + "</u>");
		}

		template <class t> void appendVector(const std::string& name, const std::vector<t>& values)
		{
			if(!loggingEnabled()) return;

			dump(getStringPrefix() + constructTable(name, std::vector<size_t>(), values));
		}		

		template <class t1, class t2> void appendMap(const std::string& name, const std::map<t1,t2>& value)
		{
			if(!loggingEnabled()) return;

			std::vector<t1> row1;
			std::vector<t2> row2;

			for (typename std::map<t1,t2>::const_iterator it = value.begin(); it != value.end(); it++)
			{
				row1.push_back(it->first);
				row2.push_back(it->second);
			}			

			dump(getStringPrefix() + constructTable(name, row1, row2));
		}
		
		void appendText(const std::string& text);
		void appendImage(const std::string& caption, const Image& img);   
		void appendMat(const std::string& caption, const cv::Mat& mat);
		void appendSegment(const std::string& name, const Segment& seg);
		void appendPoints(const std::string& name, const Points2i& pts);
		
		std::string generateImageName(std::string* html_name = NULL);
		void appendImageFile(const std::string& caption, const std::string& file);   

		void appendGraph(const Settings& vars, const std::string& name, const segments_graph::SegmentsGraph& g);
		void appendSkeleton(const Settings& vars, const std::string& name, const Skeleton::SkeletonGraph& g);   
		void appendSegmentWithYLine(const Settings& vars, const std::string& name, const Segment& seg, int line_y);

		void enterFunction(const std::string& name);
		void leaveFunction();

	private:
		FILE* FileOutput;
		bool enabled;
		bool UseVirtualFS;
		VirtualFS* pVFS;
		std::string Folder;
		size_t ImgIdent, CallIdent;
		std::vector<FunctionRecord> Stack;
		std::map<std::string, ProfilingInformation> Profile;

		void appendImageInternal(const std::string& caption, const Image& img);   
		std::string generateAnchor(const std::string& name);		
		std::string getStringPrefix(bool paragraph = false) const;
		std::string filterHtml(const std::string source) const;
		void dump(const std::string& data);
		void dumpImage(const std::string& filename, const Image& data);

		template <class t1, class t2> std::string constructTable(const std::string& caption, 
			                                                     const std::vector<t1>& row1, 
																 const std::vector<t2>& row2)
		{
			std::ostringstream header;
			header << "<table style=\"display:inline;\"><thead>";
			header << "<tr><th colspan=\"";
			header << std::min(row1.size(), row2.size());
			header << "\" align=\"left\">" + filterHtml(caption);
			header << "</th></tr></thead><tbody>";

			std::string table = header.str();
		
			if (!row1.empty())
			{
				table += "<tr>";
				for (size_t pos = 0; pos < row1.size(); pos++)
				{
					std::ostringstream visual;
					visual << row1[pos];
					table += "<td>" + filterHtml(visual.str()) + "</td>";
				}
				table += "</tr>";
			}

			if (!row2.empty())
			{
				table += "<tr>";
				for (size_t pos = 0; pos < row2.size(); pos++)
				{
					std::ostringstream visual;
					visual << row2[pos];
					table += "<td>" + filterHtml(visual.str()) + "</td>";
				}
				table += "</tr>";
			}

			table += "</tbody></table>";

			return table;
		}		
	}; /// end class log_ext
   

	namespace log_ext_service
	{
		class LogEnterFunction
		{
		public:
			LogEnterFunction(const std::string& name, log_ext& log) : Log(log)
			{
				Log.enterFunction(name);
			}
			~LogEnterFunction()
			{
				Log.leaveFunction();
			}
			void _logEnterFunction() // fake stub method for macros calling decoration
			{
			}
		private:
			log_ext& Log;
		};
	}
	
} // end namespace

#endif // _log_ext_h

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

#include "settings.h"
#include "platform_tools.h"
#include "log_ext.h"
#include "scanner.h"
#include <stdio.h>
#include <string.h> // memset
#include <algorithm>

namespace imago
{
	imago::DynamicEstimationSettings::DynamicEstimationSettings()
	{
		// default filling
		AvgBondLength = CapitalHeight = LineThickness = -1.0;
	}

	imago::GeneralSettings::GeneralSettings()
	{
		LogEnabled = LogVFSEnabled = ExtractCharactersOnly = false;
		UseProbablistics = false;
		OriginalImageWidth = OriginalImageHeight = ImageWidth = ImageHeight = 0;
		ImageAlreadyBinarized = false; // we don't know yet
		ClusterIndex = 0; // default
		StartTime = TimeLimit = 0;
		ExpandAbbreviations = true;
	}

	imago::Settings::Settings()
	{
		const char pattern = 0x6F;

		{ // initial fill
			#define APPLY(x) memset(&x, pattern, sizeof(x));

			APPLY(prefilterCV);
			APPLY(molecule);
			APPLY(estimation);
			APPLY(main);
			APPLY(mbond);
			APPLY(skeleton);
			APPLY(routines);
			APPLY(weak_seg);
			APPLY(wbe);
			APPLY(characters);
			APPLY(csr);
			APPLY(graph);
			APPLY(utils);
			APPLY(separator);
			APPLY(labels);
			APPLY(lcomb);
			APPLY(p_estimator);
			APPLY(lab_remover);
			APPLY(retinex);

			#undef APPLY
		}

		#include "settings_defaults.inc"

		{ // validation
			#define APPLY(x) \
				for (int n = 0; (n+3) < sizeof(x); n += 4) \
					if ( ((char*)(&x))[n+0] == pattern && ((char*)(&x))[n+1] == pattern && \
					     ((char*)(&x))[n+2] == pattern && ((char*)(&x))[n+3] == pattern) \
			{ \
			    printf("Uninitialized settings data in '%s' at offset %u\n", (#x), n);  \
				throw ImagoException("Uninitialized settings data at offset " + ImagoException::str(n)); \
		    }

			APPLY(prefilterCV);
			APPLY(molecule);
			APPLY(estimation);
			APPLY(main);
			APPLY(mbond);
			APPLY(skeleton);
			APPLY(routines);
			APPLY(weak_seg);
			APPLY(wbe);
			APPLY(characters);
			APPLY(csr);
			APPLY(graph);
			APPLY(utils);
			APPLY(separator);
			APPLY(labels);
			APPLY(lcomb);
			APPLY(p_estimator);
			APPLY(lab_remover);
			APPLY(retinex);

			#undef APPLY
		}
	}
	
	void imago::Settings::_fillReferenceMap(ReferenceAssignmentMap& entries)
	{
		#define STRINGIZE_NX(A)   #A
		#define STRINGIZE(A)      STRINGIZE_NX(A)
		#define ASSIGN_REF(X)     entries[ (std::string)STRINGIZE(X) ] = DataTypeReference(X);

		ASSIGN_REF(_configVersion);
		ASSIGN_REF(general.ClusterIndex);
		ASSIGN_REF(general.ImageAlreadyBinarized);

		// DO NOT FORGET TO ADD REFERENCES TO ALL NEW VARIABLES HERE!

		ASSIGN_REF(characters.MinEndpointsPossible);
		ASSIGN_REF(characters.InternalBinarizationThreshold);
		ASSIGN_REF(characters.DistanceScaleFactor);
		ASSIGN_REF(characters.PossibleCharacterDistanceStrong);
		ASSIGN_REF(characters.PossibleCharacterDistanceWeak);
		ASSIGN_REF(characters.PossibleCharacterMinimalQuality);
		ASSIGN_REF(characters.DistanceAbsolutelySure);
		ASSIGN_REF(characters.HeightMinBound);
		ASSIGN_REF(characters.HeightMaxBound);
		ASSIGN_REF(characters.ReestimateMinimalCharacters);
		ASSIGN_REF(characters.MinimalRecognizableHeight);
		ASSIGN_REF(characters.RatioDiffThresh);

		ASSIGN_REF(csr.DeleteBadTriangles);
		ASSIGN_REF(csr.Dissolve);
		ASSIGN_REF(csr.LineVectorizationFactor);
		ASSIGN_REF(csr.UseDPApproximator);
		ASSIGN_REF(csr.WeakSegmentatorDist);				
		ASSIGN_REF(csr.SmallImageDim);
		ASSIGN_REF(csr.ReconnectMinBads);
		ASSIGN_REF(csr.ReconnectSurelyBadCoef);
		ASSIGN_REF(csr.ReconnectSurelyGoodCoef);
		ASSIGN_REF(csr.ReconnectProbablyGoodCoef);
		ASSIGN_REF(csr.StableDecorner);
		ASSIGN_REF(csr.RescaleImageDimensions);

		ASSIGN_REF(estimation.CapitalHeightError);
		ASSIGN_REF(estimation.CharactersSpaceCoeff);
		ASSIGN_REF(estimation.DoubleBondDist);		
		ASSIGN_REF(estimation.MaxSymbolHeightPercentsOfImage);
		ASSIGN_REF(estimation.MaxSymRatio);
		ASSIGN_REF(estimation.MinSymRatio);
		ASSIGN_REF(estimation.ParLinesEps);
		ASSIGN_REF(estimation.SegmentVerEps);
		ASSIGN_REF(estimation.SymHeightErr);

		ASSIGN_REF(graph.MinimalDistTresh);
		ASSIGN_REF(graph.RatioSub);
		ASSIGN_REF(graph.RatioTresh);

		ASSIGN_REF(labels.adjustAttemptsCount);
		ASSIGN_REF(labels.adjustDec);
		ASSIGN_REF(labels.adjustInc);
		ASSIGN_REF(labels.ratioCapital);
		ASSIGN_REF(labels.capitalAdjustFactor);
		ASSIGN_REF(labels.ratioWeight);
		ASSIGN_REF(labels.underlinePos);
		ASSIGN_REF(labels.weightUnderline);
		
		ASSIGN_REF(lab_remover.CenterShiftMax);
		ASSIGN_REF(lab_remover.HeightFactor);
		ASSIGN_REF(lab_remover.MaxCapitalHeight);
		ASSIGN_REF(lab_remover.MaxLabelLines);
		ASSIGN_REF(lab_remover.MinCapitalHeight);
		ASSIGN_REF(lab_remover.MinimalDensity);
		ASSIGN_REF(lab_remover.MinLabelChars);
		ASSIGN_REF(lab_remover.PixGapX);
		ASSIGN_REF(lab_remover.PixGapY);

		ASSIGN_REF(lcomb.MaximalDistanceFactor);
		ASSIGN_REF(lcomb.MaximalYDistanceFactor);

		ASSIGN_REF(main.DissolvingsFactor);
		ASSIGN_REF(main.WarningsRecalcTreshold);
		ASSIGN_REF(main.MinGoodCharactersSize);
		ASSIGN_REF(main.WarningsForTooSmallCharacters);

		ASSIGN_REF(mbond.Case1Factor);
		ASSIGN_REF(mbond.Case1LengthTresh);
		ASSIGN_REF(mbond.Case2Factor);
		ASSIGN_REF(mbond.Case2LengthTresh);
		ASSIGN_REF(mbond.Case3Factor);
		ASSIGN_REF(mbond.DefaultErr);
		ASSIGN_REF(mbond.DoubleCoef);
		ASSIGN_REF(mbond.DoubleLeftLengthTresh);
		ASSIGN_REF(mbond.DoubleMagic1);
		ASSIGN_REF(mbond.DoubleMagic2);
		ASSIGN_REF(mbond.DoubleRatioTresh);
		ASSIGN_REF(mbond.DoubleRightLengthTresh);
		ASSIGN_REF(mbond.DoubleTreshMax);
		ASSIGN_REF(mbond.DoubleTreshMin);
		ASSIGN_REF(mbond.LongBond);
		ASSIGN_REF(mbond.LongErr);
		ASSIGN_REF(mbond.MaxLen1);
		ASSIGN_REF(mbond.MaxLen2);
		ASSIGN_REF(mbond.MaxLen3);
		ASSIGN_REF(mbond.MaxLen4);
		ASSIGN_REF(mbond.MaxLen5);
		ASSIGN_REF(mbond.mbe1);
		ASSIGN_REF(mbond.mbe2);
		ASSIGN_REF(mbond.mbe3);
		ASSIGN_REF(mbond.mbe4);
		ASSIGN_REF(mbond.mbe5);
		ASSIGN_REF(mbond.mbe6);
		ASSIGN_REF(mbond.mbe7);
		ASSIGN_REF(mbond.mbe_def);
		ASSIGN_REF(mbond.MediumBond);
		ASSIGN_REF(mbond.MediumErr);
		ASSIGN_REF(mbond.MinLen1);
		ASSIGN_REF(mbond.MinLen2);
		ASSIGN_REF(mbond.ParBondsEps);
		ASSIGN_REF(mbond.TripleLeftLengthTresh);
		ASSIGN_REF(mbond.TripleRightLengthTresh);

		ASSIGN_REF(molecule.AngleTreshold);
		ASSIGN_REF(molecule.LengthFactor_default);
		ASSIGN_REF(molecule.LengthFactor_long);
		ASSIGN_REF(molecule.LengthFactor_medium);
		ASSIGN_REF(molecule.LengthValue_long);
		ASSIGN_REF(molecule.LengthValue_medium);
		ASSIGN_REF(molecule.SpaceMultiply);

		ASSIGN_REF(prefilterCV.BinarizerFrameGap);
		ASSIGN_REF(prefilterCV.BinarizerThreshold);
		ASSIGN_REF(prefilterCV.BorderPartProportion);
		ASSIGN_REF(prefilterCV.MaxBadToGoodRatio);
		ASSIGN_REF(prefilterCV.MaxNonBWPixelsProportion);
		ASSIGN_REF(prefilterCV.MaxRectangleCropLineWidth);
		ASSIGN_REF(prefilterCV.MaxRectangleCropLineWidthAlreadyBinarized);
		ASSIGN_REF(prefilterCV.MinGoodPixelsCount);
		ASSIGN_REF(prefilterCV.StrongBinarizeSize);
		ASSIGN_REF(prefilterCV.StrongBinarizeTresh);
		ASSIGN_REF(prefilterCV.UseOtsuPixelsAddition);
		ASSIGN_REF(prefilterCV.OtsuThresholdValue);
		ASSIGN_REF(prefilterCV.WeakBinarizeSize);
		ASSIGN_REF(prefilterCV.WeakBinarizeTresh);

		ASSIGN_REF(p_estimator.ApriorProb4SymbolCase);
		ASSIGN_REF(p_estimator.DefaultApriority);
		ASSIGN_REF(p_estimator.LogisticLocation);
		ASSIGN_REF(p_estimator.LogisticScale);
		ASSIGN_REF(p_estimator.MinRatio2ConsiderGrPr);
		ASSIGN_REF(p_estimator.UsePerimeterNormalization);

		ASSIGN_REF(routines.Algebra_IntersectionEps);
		ASSIGN_REF(routines.Algebra_SameLineEps);
		ASSIGN_REF(routines.Circle_AsCharFactor);
		ASSIGN_REF(routines.Circle_GapAngleMax);
		ASSIGN_REF(routines.Circle_GapRadiusMax);
		ASSIGN_REF(routines.Circle_MaxDeviation);
		ASSIGN_REF(routines.Circle_MinRadius);
		ASSIGN_REF(routines.LineThick_Grid);
		
		ASSIGN_REF(separator.capHeightMax);
		ASSIGN_REF(separator.capHeightMin);
		ASSIGN_REF(separator.capHeightRatio);
		ASSIGN_REF(separator.capHeightRatio2);
		ASSIGN_REF(separator.ext2charRatio);
		ASSIGN_REF(separator.extCapHeightMax);
		ASSIGN_REF(separator.extCapHeightMin);
		ASSIGN_REF(separator.extRatioMax);
		ASSIGN_REF(separator.extRatioMin);
		ASSIGN_REF(separator.gdConst);
		ASSIGN_REF(separator.getRatio1);
		ASSIGN_REF(separator.getRatio2);
		ASSIGN_REF(separator.hu_0_1);
		ASSIGN_REF(separator.hu_0_2);
		ASSIGN_REF(separator.hu_1_1);
		ASSIGN_REF(separator.hu_1_2);
		ASSIGN_REF(separator.hu_1_3);
		ASSIGN_REF(separator.ltFactor1);
		ASSIGN_REF(separator.maxDensity);
		ASSIGN_REF(separator.minApproxSegsStrong);
		ASSIGN_REF(separator.minApproxSegsWeak);
		ASSIGN_REF(separator.minDensity);
		ASSIGN_REF(separator.specialSegmentsTreat);
		ASSIGN_REF(separator.testSlashLine1);
		ASSIGN_REF(separator.testSlashLine2);
		ASSIGN_REF(separator.UseVoteArray);
		
		ASSIGN_REF(skeleton.BaseMult);
		ASSIGN_REF(skeleton.BaseSmallErr);
		ASSIGN_REF(skeleton.BrokenRepairAngleEps);
		ASSIGN_REF(skeleton.BrokenRepairCoef1);
		ASSIGN_REF(skeleton.BrokenRepairCoef2);
		ASSIGN_REF(skeleton.BrokenRepairFactor);
		ASSIGN_REF(skeleton.ConnectBlockS);
		ASSIGN_REF(skeleton.ConnectFactor);
		ASSIGN_REF(skeleton.Dissolve2Const);
		ASSIGN_REF(skeleton.DissolveConst);
		ASSIGN_REF(skeleton.DissolveMinErr);
		ASSIGN_REF(skeleton.DistTreshLimFactor);
		ASSIGN_REF(skeleton.Join2Const);
		ASSIGN_REF(skeleton.Join3Const);
		ASSIGN_REF(skeleton.JoinVerticiesConst);
		ASSIGN_REF(skeleton.LongBondLen);
		ASSIGN_REF(skeleton.LongMul);
		ASSIGN_REF(skeleton.LongSmallErr);
		ASSIGN_REF(skeleton.MediumBondLen);
		ASSIGN_REF(skeleton.MediumMul);
		ASSIGN_REF(skeleton.MediumSmallErr);
		ASSIGN_REF(skeleton.ShortBondLen);
		ASSIGN_REF(skeleton.ShortMul);
		ASSIGN_REF(skeleton.ShrinkEps);
		ASSIGN_REF(skeleton.SlopeFact1);
		ASSIGN_REF(skeleton.SlopeFact2);

		ASSIGN_REF(utils.SlashLineDensity);

		ASSIGN_REF(wbe.MinimalSingleDownSegsCount);
		ASSIGN_REF(wbe.PointsCompareDist);
		ASSIGN_REF(wbe.SingleDownAngleMax);
		ASSIGN_REF(wbe.SingleDownCompareDist);
		ASSIGN_REF(wbe.SingleDownDistancesMax);
		ASSIGN_REF(wbe.SingleDownEps);
		ASSIGN_REF(wbe.SingleDownLengthMax);
		ASSIGN_REF(wbe.SingleUpSlopeThresh);
		ASSIGN_REF(wbe.SingleUpThickThresh);
		ASSIGN_REF(wbe.SingleUpDefCoeff);
		ASSIGN_REF(wbe.SingleUpIncCoeff);
		ASSIGN_REF(wbe.SingleUpIncLengthTresh);
		ASSIGN_REF(wbe.SingleUpInterpolateEps);
		ASSIGN_REF(wbe.SingleUpMagicAddition);
		ASSIGN_REF(wbe.SomeTresh);

		ASSIGN_REF(weak_seg.RectangularCropAreaTreshold);
		ASSIGN_REF(weak_seg.RectangularCropFitTreshold);

		ASSIGN_REF(retinex.ContrastDropPercentage);
		ASSIGN_REF(retinex.ContrastNominal);
		ASSIGN_REF(retinex.EndIteration);
		ASSIGN_REF(retinex.IterationStep);
		ASSIGN_REF(retinex.StartIteration);
	}

	bool imago::Settings::fillFromDataStream(const std::string& data)
	{
		logEnterFunction();

		int ok_vars = 0, bad_vars = 0;

		ReferenceAssignmentMap entries;
		_fillReferenceMap(entries);

		std::vector<std::string> lines;
		std::string buffer;
		for (size_t u = 0; u < data.size(); u++)
		{
			char c = data[u];
			if (c == 10) // LF
			{
				lines.push_back(buffer);
				buffer = "";
			}
			else if (c > 32)
			{
				buffer += c;
			}
		}
		if (!buffer.empty())
			lines.push_back(buffer);

		for (size_t u = 0; u < lines.size(); u++)
		{
			std::string line = lines[u];

			// cut lines after ';'
			{
				size_t p1 = line.find(';');
				if (p1 != std::string::npos)
					line = line.substr(0, p1);				
			}

			size_t p = line.find('=');
			if (p != std::string::npos)
			{
				std::string variable = line.substr(0, p);
				std::string value = line.substr(p+1);

				// printf("[%s][%s]\n", variable.c_str(), value.c_str());

				if (entries.find(variable) != entries.end())
				{
					// parse value
					if (value.find('.') != std::string::npos) // double?
					{
						if (entries[variable].getType() == DataTypeReference::otDouble)
						{
							*entries[variable].getDouble() = atof(value.c_str());
							ok_vars++;
						}
						else
						{
							getLogExt().append("Double value not expected for " + variable, value);
							bad_vars++;
						}
					}
					else
					{
						int v = atoi(value.c_str());
						if (entries[variable].getType() == DataTypeReference::otInt)
						{
							*entries[variable].getInt() = v;
							ok_vars++;
						}
						else if (entries[variable].getType() == DataTypeReference::otBool)
						{
							*entries[variable].getBool() = (v != 0);
							ok_vars++;
						}
						else
						{
							getLogExt().append("Value not expected for " + variable, value);
							bad_vars++;
						}
					}
				}
				else
				{
					getLogExt().append("Unknown variable from config", variable);
					bad_vars++;
				}
			}
		}

		getLogExt().append("Loaded ok", ok_vars);
		getLogExt().append("Errors", bad_vars);

		return ok_vars > 0; // ? (bad_vars == 0)?
	}

	void imago::Settings::saveToDataStream(std::string& data)
	{
		logEnterFunction();

		data = "";

		ReferenceAssignmentMap entries;
		_fillReferenceMap(entries);		
		
		for (ReferenceAssignmentMap::const_iterator it = entries.begin(); it != entries.end(); ++it)
		{			
			char buffer[MAX_TEXT_LINE];

			switch (it->second.getType())
			{
			case DataTypeReference::otBool:
				sprintf(buffer, "%s = %d;", it->first.c_str(), *(it->second.getBool()));
				break;
			case DataTypeReference::otInt:
				sprintf(buffer, "%s = %i;", it->first.c_str(), *(it->second.getInt()));
				break;
			case DataTypeReference::otDouble:
				sprintf(buffer, "%s = %f;", it->first.c_str(), *(it->second.getDouble()));
				break;
			}

			data += buffer + platform::getLineEndings();
		}
	}

	imago::RecognitionCaches::RecognitionCaches()
	{
		PCacheSymbolsRecognition = new RecognitionDistanceCacheType();
	}

	imago::RecognitionCaches::~RecognitionCaches()
	{
		if (PCacheSymbolsRecognition)
		{
			delete PCacheSymbolsRecognition;
			PCacheSymbolsRecognition = NULL;
		}
	}

	bool imago::Settings::forceSelectCluster(const std::string& clusterFileName)
	{		
		logEnterFunction();
		getLogExt().append("File", clusterFileName);

		try
		{
			FileScanner input("%s", clusterFileName.c_str());

			std::string stream;
			input.readAll(stream);

			return fillFromDataStream(stream);
		}
		catch(FileNotFoundException&)
		{
			getLogExt().append("Can not open config file", clusterFileName);
		}

		return false;
	}

	bool imago::Settings::checkTimeLimit() const
	{
		if (!general.TimeLimit || !general.StartTime)
			return false;
		else
			return (int)(platform::TICKS() - general.StartTime) > general.TimeLimit;
	}

	bool imago::Settings::checkTimeLimit()
	{
		if (general.TimeLimit)
		{
			if (!general.StartTime)
				general.StartTime = platform::TICKS();
			return (int)(platform::TICKS() - general.StartTime) > general.TimeLimit;
		}
		return false;
	}

	void imago::Settings::selectBestCluster()
	{
		logEnterFunction();

		/* TODO: clusters auto-detection logic
			int longestSide = std::max(general.OriginalImageWidth, general.OriginalImageHeight);
			...
			general.ClusterIndex = ct;
		}*/

	}
}

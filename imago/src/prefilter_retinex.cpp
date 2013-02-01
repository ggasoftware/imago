#include "prefilter_retinex.h"
#include <opencv2/opencv.hpp>
#include "log_ext.h"
#include "image_utils.h"
#include "prefilter_basic.h"
#include "prefilter_entry.h"

namespace imago
{
	namespace prefilter_retinex
	{
		template <typename data_t>
		double contrastNormalizeSingleIteration(std::vector<data_t>& data, double drop_percentage = 0.01, double max_average = 128)
		{
			logEnterFunction();

			const int max_value = 255;	

			int hist[max_value+1] = {0};
			int min_v = 0;
			int max_v = max_value;
			double average = 0;

			for (size_t u = 0; u < data.size(); u++)
			{
				double v = data[u];
				hist[imago::round(v)]++;
				average += v;
			}

			average /= data.size();

			if (average > max_average)
			{
				getLogExt().append("Average value is OK", average);
				return average;
			}

			{
				int sum = 0;
				while (sum < data.size() * drop_percentage && max_v > min_v) 
				{
					sum += hist[max_v];
					max_v--;
				}
			}

			{
				int sum = 0;
				while (sum < data.size() * drop_percentage && max_v > min_v) 
				{
					sum += hist[min_v];
					min_v++;
				}
			}	

			min_v = -min_v;
			max_v = max_value + (max_value - max_v);
			getLogExt().append("Normalize range min", min_v);
			getLogExt().append("Normalize range max", max_v);
			getLogExt().append("Normalized average", average);

			cv::normalize(data, data, min_v, max_v, cv::NORM_MINMAX);

			average = 0;
			for (size_t u = 0; u < data.size(); u++)
			{
				data_t& v = data[u];
				if (v < 0)
					v = 0;
				else if (v > max_value)
					v = max_value;
				average += v;
			}
			average /= data.size();
			return average;
		}


		template <typename data_t>
		void contrastNormalize(std::vector<data_t>& data, int contrastNominal, double dropPercentage)
		{
			logEnterFunction();
			
			cv::normalize(data, data, 0, 255, cv::NORM_MINMAX);
			double prev_value = 0;
			for (int iters = 0; iters < 10; iters++)
			{
				double value = contrastNormalizeSingleIteration(data, dropPercentage, contrastNominal);
				if (value > contrastNominal || value < prev_value)
					break;
				prev_value = value;
			}
		}

		template <typename data_t>
		bool computeDLT(std::vector<data_t>& data_out, const std::vector<data_t>& data_in, size_t nx, size_t ny, double t)
		{
			logEnterFunction();

			const data_t* ptr_in = &data_in.at(0);
			const data_t* ptr_in_xm1 = ptr_in - 1;
			const data_t* ptr_in_xp1 = ptr_in + 1;
			const data_t* ptr_in_ym1 = ptr_in - nx;
			const data_t* ptr_in_yp1 = ptr_in + nx;
			data_t* ptr_out = &data_out.at(0);

			data_t diff = 0.0;

			for (size_t j = 0; j < ny; j++) 
			{
				for (size_t i = 0; i < nx; i++) 
				{
					*ptr_out = 0.;
					if (0 < i) {
						diff = *ptr_in - *ptr_in_xm1;
						if (fabs(diff) > t)
							*ptr_out += diff;
					}
					if (nx - 1 > i) {
						diff = *ptr_in - *ptr_in_xp1;
						if (fabs(diff) > t)
							*ptr_out += diff;
					}
					if (0 < j) {
						diff = *ptr_in - *ptr_in_ym1;
						if (fabs(diff) > t)
							*ptr_out += diff;
					}
					if ( ny - 1 > j) {
						diff = *ptr_in - *ptr_in_yp1;
						if (fabs(diff) > t)
							*ptr_out += diff;
					}
					ptr_in++;
					ptr_in_xm1++;
					ptr_in_xp1++;
					ptr_in_ym1++;
					ptr_in_yp1++;
					ptr_out++;
				}
			}

			return true;
		}

		template <typename data_t>
		bool fastFillCosTable(size_t size, std::vector<data_t>& table)
		{   
			logEnterFunction();

			table.resize(size);

			data_t* ptr_table = &table.at(0);
			for (size_t i = 0; i < size; i++)
				*ptr_table++ = cos((data_t)(imago::PI) * i / size);

			return true;
		}

		template <typename data_t>
		bool retinexPoissonDCT(std::vector<data_t>& data, size_t nx, size_t ny, data_t m)
		{
			logEnterFunction();

			std::vector<data_t> cosi, cosj;
			fastFillCosTable(nx, cosi);
			fastFillCosTable(ny, cosj);

			data_t two = static_cast<data_t>(2.0);
			data_t m2 = static_cast<data_t>(m) / two;

			data_t* ptr_data = &data.at(0);
			data_t* ptr_cosi = &cosi.at(0);
			data_t* ptr_cosj = &cosj.at(0);

			*ptr_data++ = 0.;
			ptr_cosi++;

			for (size_t i = 1; i < nx; i++)
			{
				*ptr_data++ *= m2 / (two - *ptr_cosi++ - *ptr_cosj);
			}

			ptr_cosj++;
			ptr_cosi = &cosi.at(0);

			for (size_t j = 1; j < ny; j++)
			{
				for (size_t i = 0; i < nx; i++)
				{
					*ptr_data++ *= m2 / (two - *ptr_cosi++ - *ptr_cosj);
				}
				ptr_cosj++;
				ptr_cosi = &cosi.at(0);
			}

			return true;
		}

		template <typename data_t>
		void cvBasedCDT(std::vector<data_t>& data, size_t nx, size_t ny, int flags = 0)
		{
			logEnterFunction();

			cv::Mat1d temp(ny, nx);			
			int idx = 0;
			for (size_t y = 0; y < ny; y++)
				for (size_t x = 0; x < nx; x++)				
					temp(y,x) = data[idx++];

			cv::dct(temp, temp, flags);

			idx = 0;
			for (size_t y = 0; y < ny; y++)
				for (size_t x = 0; x < nx; x++)				
					data[idx++] = (data_t)temp(y,x);
		}


		template <typename data_t>
		bool retinexProcess(std::vector<data_t>& data, size_t nx, size_t ny, double thresh)
		{
			logEnterFunction();
			std::vector<data_t> temp(data.size());

			getLogExt().appendText("Compute discrete laplacian threshold");
			computeDLT(temp, data, nx, ny, thresh); // data -> temp

			getLogExt().appendText("Compute simple discrete cosine transform");
			cvBasedCDT(temp, nx, ny);
 
			getLogExt().appendText("Solve the Poisson PDE in Fourier space");
			data_t normDCT = (data_t)(1.0 / (data_t)(nx * ny));			
			retinexPoissonDCT(temp, nx, ny, normDCT);
	
			getLogExt().appendText("Compute inversed discrete cosine transform");
			cvBasedCDT(temp, nx, ny, cv::DCT_INVERSE);
			
			data = temp; // temp -> data

			return true;
		}		

		bool prefilterRetinexDownscaleOnly(Settings& vars, Image& raw)
		{
			return PrefilterUtils::resampleImage(vars, raw) &&
				   prefilterRetinexFullsize(vars, raw);
		}

		bool prefilterRetinexFullsize(Settings& vars, Image& raw)
		{
			logEnterFunction();

			getLogExt().appendImage("Source image", raw);

			// dimensions should be even for the FFT transform
			int width = (raw.getWidth() / 2) * 2;
			int height = (raw.getHeight() / 2) * 2;

			typedef std::vector<float> Array;
			
			Array result(width * height);

			// process multi-scale retinex
			{
				Array input(width * height);

				for (int y = 0; y < height; y++)
					for (int x = 0; x < width; x++)
						input[y * width + x] = raw.getByte(x, y);

				for (int iteration =  vars.retinex.StartIteration; 
					     iteration <  vars.retinex.EndIteration; 
						 iteration += vars.retinex.IterationStep)
				{
					getLogExt().append("Iteration", iteration);

					Array temp = input;
					double threshold = iteration;
					retinexProcess(temp, width, height, threshold);
					for (size_t u = 0; u < temp.size(); u++)
						result[u] += temp[u];
				}
			}

			// normalize contrast
			contrastNormalize(result, vars.retinex.ContrastNominal, vars.retinex.ContrastDropPercentage);

			// store result back
			getLogExt().appendText("Store image data");

			raw.clear();
			raw.init(width, height);
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					int c = imago::round(result[y * width + x]);
					if (c < 0)
						c = 0;
					else if (c > 255)
						c = 255;
					raw.getByte(x, y) = c;
				}
			}

			getLogExt().appendImage("Retinex-processed image", raw);

			// call the basic prefilter
			prefilter_basic::prefilterBasicFullsize(vars, raw);
			
			return true;
		}
	}
}
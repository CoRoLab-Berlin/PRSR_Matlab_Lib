classdef interface < matlab.System & coder.ExternalDependency & codertarget.raspi.internal.Hardware
    
    properties
        UpdateRateInHertz = 200.0;
    end

    methods
        function obj = imu_data(varargin)
            coder.allowpcode('plain');
            
            setProperties(obj,nargin,varargin{:});
        end
    end
    
    methods(Access = protected)
        function setupImpl(obj)
            if coder.target('rtw')
                coder.cinclude('main.h');
                coder.ceval('imu_init', obj.UpdateRateInHertz);
                coder.ceval('spi_init');
            elseif ( coder.target('Sfun') )
                %
            end
        end

        function [dt1, acc1_x, acc1_y, acc1_z, gyr1_x, gyr1_y, gyr1_z, ...
                  dt2, acc2_x, acc2_y, acc2_z, gyr2_x, gyr2_y, gyr2_z, ...
                  adcValues0, adcValues1, adcValues2, adcValues3, ...
                  adcValues4, adcValues5, adcValues6, adcValues7] = stepImpl(~, DAC_0, DAC_1, DAC_2, DAC_3)
            
            imu_out = int32(ones(14,1));
            spi_out = zeros(8,1);

            Max = 10;
            Min = 0;
            
            if coder.target('rtw')               
                DAC_0(DAC_0>Max) = Max;
                DAC_0(DAC_0<Min) = Min;
                DAC_1(DAC_1>Max) = Max;
                DAC_1(DAC_1<Min) = Min;
                DAC_2(DAC_2>Max) = Max;
                DAC_2(DAC_2<Min) = Min;
                DAC_3(DAC_3>Max) = Max;
                DAC_3(DAC_3<Min) = Min;

                in = [DAC_0,DAC_1,DAC_2,DAC_3];

                coder.cinclude('main.h');
                coder.ceval('step', coder.wref(imu_out), coder.ref(in), coder.wref(spi_out));
            elseif ( coder.target('Sfun') )
                imu_out = int32(nan(14,1));
                spi_out = nan(8,1);
            end

            dt1    = imu_out(1);
            acc1_x = imu_out(2);
            acc1_y = imu_out(3);
            acc1_z = imu_out(4);
            gyr1_x = imu_out(5);
            gyr1_y = imu_out(6);
            gyr1_z = imu_out(7);
            dt2    = imu_out(8);
            acc2_x = imu_out(9);
            acc2_y = imu_out(10);
            acc2_z = imu_out(11);
            gyr2_x = imu_out(12);
            gyr2_y = imu_out(13);
            gyr2_z = imu_out(14);

            adcValues0 = spi_out(1);
            adcValues1 = spi_out(2);
            adcValues2 = spi_out(3);
            adcValues3 = spi_out(4);
            adcValues4 = spi_out(5);
            adcValues5 = spi_out(6);
            adcValues6 = spi_out(7);
            adcValues7 = spi_out(8);

        end
        
        function releaseImpl(~)
        end
    end
    
    methods (Static)
        function name = getDescriptiveName()
            name = 'interface';
        end
        
        function b = isSupportedContext(context)
            b = context.isCodeGenTarget('rtw');
        end
        
        function updateBuildInfo(buildInfo, context)
            if context.isCodeGenTarget('rtw')
                rootDir = fullfile(fileparts(mfilename('fullpath')));
                buildInfo.addIncludePaths(rootDir);
                
                addSourceFiles(buildInfo,'bmi270.c', rootDir);
                addSourceFiles(buildInfo,'hw_spi.c', rootDir);
                addSourceFiles(buildInfo,'main.c', rootDir);
            end
        end
    end
end

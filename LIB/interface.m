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
            elseif ( coder.target('Sfun') )
                %
            end
        end

        function [dt1, acc1_x, acc1_y, acc1_z, gyr1_x, gyr1_y, gyr1_z, ...
                  dt2, acc2_x, acc2_y, acc2_z, gyr2_x, gyr2_y, gyr2_z] = stepImpl(~)
            
            imu_out = double(ones(14,1));
            
            if coder.target('rtw')               
                coder.cinclude('main.h');
                coder.ceval('step', coder.wref(imu_out));
            elseif ( coder.target('Sfun') )
                imu_out = double(nan(14,1));
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
                addSourceFiles(buildInfo,'main.c', rootDir);
            end
        end
    end
end

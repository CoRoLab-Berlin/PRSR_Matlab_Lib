classdef hwdac < matlab.System & coder.ExternalDependency  & codertarget.raspi.internal.Hardware
% ExternalCCode Compute output by calling into external C Code
    methods 
        % Constructor 
        function obj = hwdac(varargin) 
            coder.allowpcode('plain'); 
            
            % Support name-value pair arguments when constructing the object. 
            setProperties(obj,nargin,varargin{:}); 
        end 
    end 
methods(Access = protected)
        function setupImpl(~)
            if coder.target('Rtw')% done only for code gen
                coder.cinclude('hw_spi.hpp');
                coder.ceval('init');
            elseif ( coder.target('Sfun') )
                %
            end
        end
        function [] = stepImpl(~, DAC_0,DAC_1,DAC_2,DAC_3)
            if coder.target('Rtw')% done only for code gen
                Max = 10;
                Min = 0;
                
                DAC_0(DAC_0>Max) = Max;
                DAC_0(DAC_0<Min) = Min;
                DAC_1(DAC_1>Max) = Max;
                DAC_1(DAC_1<Min) = Min;
                DAC_2(DAC_2>Max) = Max;
                DAC_2(DAC_2<Min) = Min;
                DAC_3(DAC_3>Max) = Max;
                DAC_3(DAC_3<Min) = Min;
               
                coder.cinclude('hw_spi.hpp');
                in = [DAC_0,DAC_1,DAC_2,DAC_3];            
                coder.ceval('setDAC', coder.ref(in));
            elseif ( coder.target('Sfun') )
                %
            end
            
            
        end
        function releaseImpl(~)

        end
    end
  

    methods (Static) 
        
        function name = getDescriptiveName() 
            name = 'hwdac'; 
        end 
        
        function b = isSupportedContext(context) 
            b = context.isCodeGenTarget('rtw'); 
        end 
        
        function updateBuildInfo(buildInfo, context) 
            if context.isCodeGenTarget('rtw')
                % Update buildInfo 
                rootDir = fullfile(fileparts(mfilename('fullpath'))); 
                buildInfo.addIncludePaths(rootDir); 
                % Use the following API's to add include files, sources and 
                addSourceFiles(buildInfo,'hw_spi.cpp',rootDir); 
            end  
        end 
    end
end

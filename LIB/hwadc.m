classdef hwadc < matlab.System & coder.ExternalDependency & codertarget.raspi.internal.Hardware
% ExternalCCode Compute output by calling into external C Code
    methods 
        % Constructor 
        function obj = hwadc(varargin) 
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
        function [adcValues0,adcValues1,adcValues2,adcValues3,adcValues4,...
                    adcValues5,adcValues6,adcValues7] = stepImpl(~)
            
            out = zeros(8,1); 
            if coder.target('Rtw')% done only for code gen
                % Add header file to build
                coder.cinclude('hw_spi.hpp');
                coder.ceval('getADC', coder.wref(out));
            elseif ( coder.target('Sfun') )
                out = nan(8,1);
            end
            adcValues0 = out(1);
            adcValues1 = out(2);
            adcValues2 = out(3);
            adcValues3 = out(4);
            adcValues4 = out(5);
            adcValues5 = out(6);
            adcValues6 = out(7);
            adcValues7 = out(8);
            
        end
           
        function releaseImpl(~)
        end
    end
  

    methods (Static) 
        
        function name = getDescriptiveName() 
            name = 'hwadc'; 
        end 
        
        function b = isSupportedContext(context) 
            b = context.isCodeGenTarget('Rtw'); 
        end 
        
        function updateBuildInfo(buildInfo, context) 
            if context.isCodeGenTarget('rtw')
                % Update buildInfo 
                rootDir = fullfile(fileparts(mfilename('fullpath'))); 
                buildInfo.addIncludePaths(rootDir); 
                % Use the following API's to add include files, sources and 
                % linker flags 
                addSourceFiles(buildInfo,'hw_spi.cpp',rootDir); 
            end  
        end 
    end
end

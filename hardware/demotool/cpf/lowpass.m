function [output_lowpass] = lowpass(B, A, resample_factor, input_lowpass)

[r c] = size(input_lowpass);

% substract offset
input_lowpass_zero = input_lowpass;
for i=1:c  
  input_lowpass_zero(:,i) = input_lowpass(:,i) - input_lowpass(1,i);  
end

% filter
output_lowpass = filter(B, A, input_lowpass_zero);

% add offset again
for i=1:c
  output_lowpass(:,i) = output_lowpass(:,i) + input_lowpass(1,i);  
end

% resample
output_lowpass = output_lowpass(1:1/resample_factor:r,:);

#!/usr/bin/env python
# coding: utf-8

# In[1]:


import json
import re


# In[9]:


def function_generator(name, return_type,  parameters):
    ret = ""
    
#     print(parameters)
    parameter_decl_list = [ param["full"] for param in parameters ]
    print(", ".join(parameter_decl_list))
    parameters = [param["analysed"] for param in parameters]
    
    ret += f"""
{return_type} {name} ({ ", ".join(parameter_decl_list) }) {{"""
    
    ret += f"""
    PRINT("Enter {name}()\\n");
    typedef decltype(&{name}) funcType;
    funcType func = (funcType) actualDlsym(libcudnnHandle, \"{name}\");
    fromCudnnApiName(\"{name}\");"""
    
    def io_check(typ):
        if typ == "const void *":
            return "Input"
        elif typ == "void *":
            return "Output"
        else:
            return None
    
    def camel_case_split(identifier):
        matches = re.finditer('.+?(?:(?<=[a-z])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])|$)', identifier)
        return {idx:m.group(0) for idx, m in enumerate(matches)}
    
    # name analysis
    namesp = camel_case_split(name)
    api_type = namesp.get(1)
    api_target = namesp.get(2)
    print(name, api_type, api_target)
    
    # memory object create/destroy
    if api_type in ["Create", "Destroy"] and api_target in ["Tensor", "Filter"]:
        ret += f"""
    fromCudnn{api_type}{api_target}Desc({api_target.lower()}Desc);
    """
    

    compute_flag = (api_type not in ["Get", "Set", "Create", "Destroy"])
    
    if compute_flag:
        ret += f"""
    prefetchPreKernel();
    inCudnnInvocation = true;
    """
    
    
    # params analysis
    params_lines = []
    for idx, param in enumerate(parameters):
        typ, pn = param
        
        if typ == "const cudnnTensorDescriptor_t" or typ == "const cudnnFilterDescriptor_t":
            ds_typ = typ[11:-12]
            
            look_forward_length = 1
            if pn == "dBnScaleBiasDesc" or "DivisiveNormalization" in name: 
                look_forward_length = 4
            
            for next_typ, next_pn in parameters[idx + 1: idx + look_forward_length + 1]:
                io_tag = io_check(next_typ)
                if io_tag:
                    params_lines.append(f"fromCudnn{io_tag}{ds_typ}({pn}, {next_pn});")
                
        elif typ == "void *": 
            look_forward_length = 1
            for next_typ, next_pn in parameters[idx + 1: idx + look_forward_length + 1]:
                if 'SizeInBytes' in next_pn:
                    params_lines.append(f"fromCudnnWorkspace({pn}, {next_pn});")
    
    ret += "\n\t".join(params_lines)
    
    parameter_list = [ param[1] for param in parameters ]
    ret += f"""
    {return_type} ret = func({", ".join(parameter_list)});"""
    
    if api_type == "Set" and api_target in ["Tensor", "Filter"]:
        ret += f"""
    fromCudnn{api_type}{api_target}DescSize({api_target.lower()}Desc);
    """
    
    
    if compute_flag:
        ret += f"""
    inCudnnInvocation = false;
    prefetchPostKernel();"""
    
    ret += f"""
    return ret;
}}"""
    return ret


# In[ ]:





# In[10]:


output = []
with open('gen.json') as f:
    for line in f:
        func_object = json.loads(line)
        output.append(function_generator(**func_object))


with open('gen.cpp', "w") as wt:
    wt.write("\n".join(output))


# In[ ]:





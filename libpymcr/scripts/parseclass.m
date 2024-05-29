function outstruct = parseclass(classname)
    if classname(1) == '@'
        classname = classname(2:end);
    end
    if classname(end-1:end) == '.m'
        classname = classname(1:end-2);
    end
    if verLessThan('Matlab', '24.1')
        classinfo = meta.class.fromName(classname);
    else
        classinfo = matlab.metadata.Class.fromName(classname);
    end
    % Ignore handle methods
    handle_methods = {'addlistener', 'delete', 'empty', 'eq', 'findobj', ...
        'findprop', 'ge', 'gt', 'isvalid', 'le', 'listener', 'lt', 'ne', 'notify'};
    [class_methods{1:numel(classinfo.MethodList)}] = deal(classinfo.MethodList.Name);
    [~, icm] = setdiff(class_methods, handle_methods);
    for ii = 1:numel(icm)
        methodobj = classinfo.MethodList(icm(ii));
        out_methods(ii) = struct('name', methodobj.Name, ...
                                 'inputs', {methodobj.InputNames}, ...
                                 'outputs', {methodobj.OutputNames}, ...
                                 'doc', evalc(['help ' classname '/' methodobj.Name]));
    end
    nonhidden = arrayfun(@(x) ~x.Hidden, classinfo.PropertyList);
    out_props = arrayfun(@(x) struct('name', x.Name, ...
                                     'doc', evalc(['help ' classname '/' x.Name])), classinfo.PropertyList(nonhidden));
    outstruct = struct('name', classname, ...
                       'methods', out_methods(:), ...
                       'properties', out_props(:), ...
                       'doc', evalc(['help ' classname]));
end
